/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/huobi/market_data.hpp"

#include <algorithm>

#include "roq/mask.hpp"
#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"
#include "roq/core/charconv.hpp"

#include "roq/core/tools/exception.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/socket/client_factory.hpp"

#include "roq/huobi/flags.hpp"

#include "roq/huobi/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace huobi {

namespace {
auto const NAME = "md"sv;
const Mask SUPPORTS{
    SupportType::TOP_OF_BOOK,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(std::string_view const &group, std::string_view const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::ws_market_uri();
  web::socket::Client::Config config{
      .always_reconnect = true,
      .connection_timeout = server::Flags::net_connection_timeout(),
      .disconnect_on_idle_timeout = server::Flags::net_disconnect_on_idle_timeout(),
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .uris = {&uri, 1},
      .query = {},
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return web::socket::ClientFactory::create(handler, context, config, []() { return std::string(); });
}

template <typename T>
void emplace(Trade &result, const T &value) {
  new (&result) Trade{
      .side = json::map(value.direction),
      .price = value.price,
      .quantity = value.amount,
      .trade_id = {},
  };
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

MarketData::MarketData(Handler &handler, io::Context &context, uint32_t stream_id, Shared &shared, size_t index)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)), index_(index),
      connection_(create_connection(*this, context)), decode_buffer_(Flags::decode_buffer_size()),
      request_id_(static_cast<uint64_t>(stream_id_) * 1000000),  // scale (debugging)
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
          .total_bytes_received = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .ping = create_metrics(name_, "ping"sv),
          .error = create_metrics(name_, "error"sv),
          .subbed = create_metrics(name_, "subbed"sv),
          .bbo = create_metrics(name_, "bbo"sv),
          .trade = create_metrics(name_, "trade"sv),
          .detail = create_metrics(name_, "detail"sv),
          .ticker = create_metrics(name_, "ticker"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
      },
      shared_(shared), inflate_(core::zlib::Inflate::GZIP_NO_HEADER), request_queue_(Flags::ws_request_delay()) {
}

void MarketData::operator()(Event<Start> const &) {
  (*connection_).start();
}

void MarketData::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void MarketData::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (ready())
    check_request_queue(now);
}

void MarketData::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      .write(counter_.total_bytes_received, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.ping, metrics::PROFILE)
      .write(profile_.error, metrics::PROFILE)
      .write(profile_.subbed, metrics::PROFILE)
      .write(profile_.bbo, metrics::PROFILE)
      .write(profile_.trade, metrics::PROFILE)
      .write(profile_.detail, metrics::PROFILE)
      .write(profile_.ticker, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void MarketData::subscribe(size_t start_from) {
  if (ready())
    subscribe(shared_.symbols.get_slice(index_, start_from));
}

void MarketData::operator()(web::socket::Client::Connected const &) {
}

void MarketData::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  request_queue_.clear();
}

void MarketData::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::READY);
  subscribe();
}

void MarketData::operator()(web::socket::Client::Close const &) {
}

void MarketData::operator()(web::socket::Client::Latency const &latency) {
  auto trace_info = server::create_trace_info();
  const ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(web::socket::Client::Text const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(web::socket::Client::Binary const &binary) {
  if (inflate_.decode(binary.payload, inflate_buffer_, [&](auto &payload) {
        std::string_view message{reinterpret_cast<char const *>(std::data(payload)), std::size(payload)};
        log::info<5>(R"(message="{}")"sv, message);
        parse(message);
      })) {
  } else {
    log::fatal("Failed to decode message"sv);
  }
  counter_.total_bytes_received.update((*connection_).total_bytes_received());
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    const StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void MarketData::subscribe(std::span<Symbol const> const &symbols) {
  if (std::empty(symbols))
    return;
  subscribe(symbols, "market"sv, "bbo"sv);
  subscribe(symbols, "market"sv, "ticker"sv);
  subscribe(symbols, "market"sv, "trade.detail"sv);
  subscribe(symbols, "market"sv, "detail"sv);
}

void MarketData::subscribe(
    std::span<Symbol const> const &symbols, std::string_view const &source, std::string_view const &theme) {
  assert(!std::empty(symbols));
  for (auto &symbol : symbols) {
    auto id = ++request_id_;
    auto message = fmt::format(
        R"({{)"
        R"("sub":"{}.{}.{}",)"
        R"("id":"{}")"
        R"(}})"sv,
        source,
        symbol,
        theme,
        id);
    request_queue_.emplace_back(message);
  }
}

void MarketData::send_pong(std::chrono::milliseconds timestamp) {
  auto message = fmt::format(
      R"({{)"
      R"("pong":{})"
      R"(}})"sv,
      timestamp.count());
  (*connection_).send_text(message);  // note! special, can't delay
}

void MarketData::parse(std::string_view const &message) {
  profile_.parse([&]() {
    try {
      auto trace_info = server::create_trace_info();
      core::json::Buffer buffer(decode_buffer_);
      json::Parser::dispatch(*this, message, buffer, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void MarketData::operator()(Trace<json::Ping const> const &event) {
  profile_.ping([&]() {
    auto &[trace_info, ping] = event;
    log::debug("ping={}"sv, ping);
    send_pong(ping.timestamp);
  });
}

void MarketData::operator()(Trace<json::Error const> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
  });
}

void MarketData::operator()(Trace<json::Subbed const> const &event) {
  profile_.subbed([&]() {
    auto &[trace_info, subbed] = event;
    log::info<1>("subbed={}"sv, subbed);
  });
}

void MarketData::operator()(Trace<json::BBO const> const &event) {
  profile_.bbo([&]() {
    auto &[trace_info, bbo] = event;
    (*connection_).touch(trace_info.source_receive_time);
    auto symbol = json::extract_symbol(bbo.ch);
    auto &tick = bbo.tick;
    const TopOfBook top_of_book{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .layer{
            .bid_price = tick.bid,
            .bid_quantity = tick.bid_size,
            .ask_price = tick.ask,
            .ask_quantity = tick.ask_size,
        },
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(bbo.ts),
        .exchange_sequence = {},
    };
    create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
  });
}

void MarketData::operator()(Trace<json::Trade const> const &event) {
  profile_.trade([&]() {
    auto &[trace_info, trade] = event;
    (*connection_).touch(trace_info.source_receive_time);
    auto symbol = json::extract_symbol(trade.ch);
    auto &tick = trade.tick;
    core::back_emplacer trades(shared_.trades);
    for (auto &item : tick.data)
      trades.emplace_back([&item](auto &result) { emplace(result, item); });
    const TradeSummary trade_summary{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .trades = trades,
        .exchange_time_utc = utils::safe_cast(trade.ts),
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
  });
}

void MarketData::operator()(Trace<json::Detail const> const &event) {
  profile_.detail([&]() {
    auto &[trace_info, detail] = event;
    (*connection_).touch(trace_info.source_receive_time);
    auto symbol = json::extract_symbol(detail.ch);
    auto &tick = detail.tick;
    Statistics statistics[] = {
        {
            .type = StatisticsType::OPEN_PRICE,
            .value = tick.open,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::HIGHEST_TRADED_PRICE,
            .value = tick.high,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::LOWEST_TRADED_PRICE,
            .value = tick.low,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::CLOSE_PRICE,
            .value = tick.close,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::TRADE_VOLUME,
            .value = tick.vol,  // note! not sure...  (amount? count?)
            .begin_time_utc = {},
            .end_time_utc = {},
        },
    };
    const StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(detail.ts),
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::Ticker const> const &event) {
  profile_.ticker([&]() {
    auto &[trace_info, ticker] = event;
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void MarketData::operator()(Trace<json::MBP const> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::MBPSnapshot const> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::check_request_queue(std::chrono::nanoseconds now) {
  request_queue_.dispatch(
      [&](auto now) { return shared_.rate_limiter.can_request(now); },
      [&](auto &message) {
        log::debug(R"(Sending request: message="{}")"sv, message);
        (*connection_).send_text(message);
      },
      now);
}

}  // namespace huobi
}  // namespace roq
