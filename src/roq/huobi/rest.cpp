/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/huobi/rest.hpp"

#include <utility>

#include "roq/mask.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"
#include "roq/core/charconv.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/rest/client_factory.hpp"

#include "roq/huobi/flags.hpp"

#include "roq/huobi/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace huobi {

namespace {
auto const NAME = "rest"sv;
const Mask SUPPORTS{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(std::string_view const &group, std::string_view const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::rest_uri();
  web::rest::Client::Config config{
      .decode_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .uris = {&uri, 1},
      .proxy = Flags::rest_proxy(),
      .user_agent = ROQ_PACKAGE_NAME,
      .connection = web::http::Connection::KEEP_ALIVE,
      .allow_pipelining = true,
      .request_timeout = Flags::rest_request_timeout(),
      .ping_frequency = Flags::rest_ping_freq(),
      .ping_path = Flags::rest_ping_path(),
  };
  return web::rest::ClientFactory::create(handler, context, config);
}

template <typename T>
void emplace(MBPUpdate &result, const T &value) {
  new (&result) MBPUpdate{
      .price = value.price,
      .quantity = value.qty,
      .implied_quantity = NaN,
      .number_of_orders = {},
      .update_action = {},
      .price_level = {},
  };
}
}  // namespace

Rest::Rest(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      connection_(create_connection(*this, context)), decode_buffer_(Flags::decode_buffer_size()),
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
      shared_(shared), download_(Flags::rest_request_timeout(), [this](auto state) { return download(state); }) {
}

void Rest::operator()(Event<Start> const &) {
  (*connection_).start();
}

void Rest::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void Rest::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
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

void Rest::operator()(web::rest::Client::Connected const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void Rest::operator()(web::rest::Client::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void Rest::operator()(web::rest::Client::Latency const &latency) {
  auto trace_info = server::create_trace_info();
  const ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void Rest::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    const StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t Rest::download(RestState state) {
  switch (state) {
    using enum RestState;
    case UNDEFINED:
      assert(false);
      break;
    case MARKET_STATUS:
      get_market_status();
      return 1;
    case CURRENCIES:
      get_currencies();
      return 1;
    case SYMBOLS:
      get_symbols();
      return 1;
    case DONE:
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
    auto method = web::http::Method::GET;
    auto path = "/v2/market-status"sv;
    web::rest::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("market_status"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      auto trace_info = server::create_trace_info();
      Trace event(trace_info, response);
      get_market_status_ack(event, sequence);
    });
  });
}

void Rest::get_market_status_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
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
      response.expect(web::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      const auto market_status = core::json::Parser::create<json::MarketStatus>(body, buffer);
      Trace event(trace_info, market_status);
      (*this)(event);
      download_.check(state);
    } catch (NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(Trace<json::MarketStatus> const &event) {
  auto &[trace_info, market_status] = event;
  log::info<2>("market_status={}"sv, market_status);
}

// currencies

void Rest::get_currencies() {
  profile_.currencies([&]() {
    auto method = web::http::Method::GET;
    auto path = "/v1/common/currencys"sv;
    web::rest::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("currencies"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      auto trace_info = server::create_trace_info();
      Trace event(trace_info, response);
      get_currencies_ack(event, sequence);
    });
  });
}

void Rest::get_currencies_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
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
      response.expect(web::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      const auto currencies = core::json::Parser::create<json::Currencies>(body, buffer);
      Trace event(trace_info, currencies);
      (*this)(event);
      download_.check(state);
    } catch (NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(Trace<json::Currencies> const &event) {
  auto &[trace_info, currencies] = event;
  log::info<2>("currencies={}"sv, currencies);
}

// symbols

void Rest::get_symbols() {
  profile_.symbols([&]() {
    auto method = web::http::Method::GET;
    auto path = "/v1/common/symbols"sv;
    web::rest::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("symbols"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      auto trace_info = server::create_trace_info();
      Trace event(trace_info, response);
      get_symbols_ack(event, sequence);
    });
  });
}

void Rest::get_symbols_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
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
      response.expect(web::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      const auto symbols = core::json::Parser::create<json::Symbols>(body, buffer);
      Trace event(trace_info, symbols);
      (*this)(event);
      download_.check(state);
    } catch (NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(Trace<json::Symbols> const &event) {
  auto &[trace_info, symbols] = event;
  log::info<2>("symbols={}"sv, symbols);
  std::vector<Symbol> symbols_2;
  size_t counter = {};
  for (auto &item : symbols.data) {
    log::debug("item={}"sv, item);
    if (item.state != json::State::ONLINE || item.api_trading != json::Trading::ENABLED) {
      log::info<1>(R"(Drop symbol="{}")"sv, item.symbol);
      continue;
    }
    auto &symbol = item.symbol;
    auto discard = shared_.discard_symbol(item.symbol);
    auto tick_size = std::pow(10.0, -item.price_precision);
    auto trade_vol_step_size = std::pow(10.0, -item.amount_precision);
    const ReferenceData reference_data{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .description = {},
        .security_type = {},
        .base_currency = item.base_currency,
        .quote_currency = item.quote_currency,
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = tick_size,
        .multiplier = NaN,
        .min_notional = NaN,
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
        .discard = discard,
    };
    create_trace_and_dispatch(handler_, trace_info, reference_data, false);
    if (discard) {
      log::info<1>(R"(Drop symbol="{}")"sv, item.symbol);
      continue;
    }
    if (all_symbols_.emplace(symbol).second)  // only include new
      symbols_2.emplace_back(symbol);
    ++counter;
    auto trading_status = json::map(item.api_trading);
    const MarketStatus market_status{
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

}  // namespace huobi
}  // namespace roq
