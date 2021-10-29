/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/huobi/rest.h"

#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"
#include "roq/core/charconv.h"

#include "roq/core/metrics/factory.h"

#include "roq/huobi/flags.h"

#include "roq/huobi/json/utils.h"

using namespace std::literals;

namespace roq {
namespace huobi {

namespace {
static const auto NAME = "rest"sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};

static const auto ALLOW_PIPELINING = true;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

template <typename T>
void emplace(MBPUpdate &result, const T &value) {
  new (&result) MBPUpdate{
      .price = value.price,
      .quantity = value.qty,
      .implied_quantity = NaN,
      .price_level = {},
      .number_of_orders = {},
  };
}
}  // namespace

Rest::Rest(Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      connection_(
          *this,
          context,
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          core::URI(Flags::rest_uri()),
          ROQ_PACKAGE_NAME,
          core::http::Connection::KEEP_ALIVE,
          ALLOW_PIPELINING,
          Flags::rest_request_timeout(),
          Flags::rest_rate_limit_interval(),
          Flags::rest_rate_limit_max_requests(),
          Flags::rest_ping_freq(),
          Flags::rest_ping_path()),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .market_status = create_metrics(name_, "market_status"sv),
          .market_status_ack = create_metrics(name_, "market_status_ack"sv),
          .currencies = create_metrics(name_, "currencies"sv),
          .currencies_ack = create_metrics(name_, "currencies_ack"sv),
          .symbols = create_metrics(name_, "symbols"sv),
          .symbols_ack = create_metrics(name_, "symbols_ack"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
      },
      shared_(shared),
      download_(Flags::rest_request_timeout(), [this](auto state) { return download(state); }) {
}

void Rest::operator()(const Event<Start> &) {
  connection_.start();
}

void Rest::operator()(const Event<Stop> &) {
  connection_.stop();
}

void Rest::operator()(const Event<Timer> &event) {
  auto now = event.value.now;
  connection_.refresh(now);
  if (ready())
    check_request_queue(now);
}

void Rest::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.market_status, metrics::PROFILE)
      .write(profile_.market_status_ack, metrics::PROFILE)
      .write(profile_.currencies, metrics::PROFILE)
      .write(profile_.currencies_ack, metrics::PROFILE)
      .write(profile_.symbols, metrics::PROFILE)
      .write(profile_.symbols_ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

void Rest::operator()(const core::web::Client::Connected &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void Rest::operator()(const core::web::Client::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void Rest::operator()(const core::web::Client::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void Rest::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::REST,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t Rest::download(RestState state) {
  switch (state) {
    case RestState::UNDEFINED:
      assert(false);
      break;
    case RestState::MARKET_STATUS:
      get_market_status();
      return 1;
    case RestState::CURRENCIES:
      get_currencies();
      return 1;
    case RestState::SYMBOLS:
      get_symbols();
      return 1;
    case RestState::DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

// market-status

void Rest::get_market_status() {
  profile_.market_status([&]() {
    auto method = core::http::Method::GET;
    auto path = "/v2/market-status"sv;
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
        .rate_limit_weight = 1,
    };
    auto sequence = download_.sequence();
    connection_(
        "market_status"sv,
        request,
        [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          server::Trace event(trace_info, response);
          get_market_status_ack(event, sequence);
        });
  });
}

void Rest::get_market_status_ack(
    const server::Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.market_status_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = RestState::MARKET_STATUS;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto market_status = core::json::Parser::create<json::MarketStatus>(body, buffer);
      server::Trace event(trace_info, market_status);
      (*this)(event);
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(const server::Trace<json::MarketStatus> &event) {
  auto &[trace_info, market_status] = event;
  log::info<2>("market_status={}"sv, market_status);
}

// currencies

void Rest::get_currencies() {
  profile_.currencies([&]() {
    auto method = core::http::Method::GET;
    auto path = "/v1/common/currencys"sv;
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
        .rate_limit_weight = 1,
    };
    auto sequence = download_.sequence();
    connection_(
        "currencies"sv,
        request,
        [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          server::Trace event(trace_info, response);
          get_currencies_ack(event, sequence);
        });
  });
}

void Rest::get_currencies_ack(const server::Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.currencies_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = RestState::CURRENCIES;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto currencies = core::json::Parser::create<json::Currencies>(body, buffer);
      server::Trace event(trace_info, currencies);
      (*this)(event);
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(const server::Trace<json::Currencies> &event) {
  auto &[trace_info, currencies] = event;
  log::info<2>("currencies={}"sv, currencies);
}

// symbols

void Rest::get_symbols() {
  profile_.symbols([&]() {
    auto method = core::http::Method::GET;
    auto path = "/v1/common/symbols"sv;
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
        .rate_limit_weight = 1,
    };
    auto sequence = download_.sequence();
    connection_(
        "symbols"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          server::Trace event(trace_info, response);
          get_symbols_ack(event, sequence);
        });
  });
}

void Rest::get_symbols_ack(const server::Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.symbols_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = RestState::SYMBOLS;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto symbols = core::json::Parser::create<json::Symbols>(body, buffer);
      server::Trace event(trace_info, symbols);
      (*this)(event);
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(const server::Trace<json::Symbols> &event) {
  auto &[trace_info, symbols] = event;
  log::info<2>("symbols={}"sv, symbols);
  std::vector<std::string> symbols_2;
  size_t counter = {};
  for (auto &item : symbols.data) {
    if (shared_.discard_symbol(item.symbol)) {
      log::info<1>(R"(Drop symbol="{}")"sv, item.symbol);
      continue;
    }
    auto &symbol = item.symbol;
    if (all_symbols_.emplace(symbol).second)  // only include new
      symbols_2.emplace_back(symbol);
    ++counter;
    auto tick_size = std::pow(10.0, -item.price_precision);
    auto trade_vol_step_size = std::pow(10.0, -item.amount_precision);
    ReferenceData reference_data{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .description = {},
        .security_type = {},
        .base_currency = item.base_currency,
        .quote_currency = item.quote_currency,
        .commission_currency = {},
        .tick_size = tick_size,
        .multiplier = NaN,
        .min_trade_vol = item.min_order_amt,
        .max_trade_vol = item.max_order_amt,
        .trade_vol_step_size = trade_vol_step_size,
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = item.underlying,
        .time_zone = {},
        .issue_date = {},
        .settlement_date = {},
        .expiry_datetime = {},
        .expiry_datetime_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, reference_data, false);
    auto trading_status = json::map(item.api_trading);
    MarketStatus market_status{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .trading_status = trading_status,
    };
    create_trace_and_dispatch(handler_, trace_info, market_status, true);
  }
  log::info("Exchange info: including symbols {}/{}"sv, counter, std::size(symbols.data));
  if (!std::empty(symbols_2)) {
    SymbolsUpdate symbols_update{
        .symbols = symbols_2,
    };
    handler_(symbols_update);
  }
}

// queue

void Rest::check_request_queue(std::chrono::nanoseconds now) {
  while (!shared_.request_queue.empty()) {
    auto &tmp = shared_.request_queue.front();
    if (now < tmp.first)
      break;
    if (shared_.can_request(now, [&]() {
          auto &symbol = tmp.second;
          log::debug(R"(Requesting order book snapshot symbol="{}")"sv, symbol);
          // XXX HANS get_depth(symbol);
          shared_.request_queue.pop_front();
        })) {
    } else {
      return;
    }
  }
}

}  // namespace huobi
}  // namespace roq
