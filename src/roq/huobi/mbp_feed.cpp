/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/huobi/mbp_feed.hpp"

#include <algorithm>

#include "roq/mask.hpp"
#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"
#include "roq/core/charconv.hpp"

#include "roq/core/tools/exception.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/huobi/flags.hpp"

#include "roq/huobi/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace huobi {

namespace {
const auto NAME = "mbp"sv;
const auto SUPPORTS = Mask{
    SupportType::MARKET_BY_PRICE,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::ws_mbp_uri();
  core::web::ClientSocket::Config config{
      .validate_certificate = server::Flags::tls_validate_certificate(),
      .uris = {&uri, 1},
      .query = {},
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return core::web::ClientSocket{handler, context, config, []() { return std::string(); }};
}

template <typename T>
void emplace(MBPUpdate &result, const T &value) {
  new (&result) MBPUpdate{
      .price = value.price,
      .quantity = value.vol,
      .implied_quantity = NaN,
      .price_level = {},
      .number_of_orders = {},
  };
}
}  // namespace

MBPFeed::MBPFeed(
    Handler &handler, core::io::Context &context, uint32_t stream_id, Shared &shared, size_t index)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      index_(index), connection_(create_connection(*this, context)),
      decode_buffer_(Flags::decode_buffer_size()),
      request_id_(static_cast<uint64_t>(stream_id_) * 1000000),  // scale (debugging)
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .ping = create_metrics(name_, "ping"sv),
          .error = create_metrics(name_, "error"sv),
          .subbed = create_metrics(name_, "subbed"sv),
          .mbp = create_metrics(name_, "mbp"sv),
          .mbp_snapshot = create_metrics(name_, "mbp_snapshot"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
      },
      shared_(shared), inflate_(core::zlib::Inflate::GZIP_NO_HEADER) {
}

void MBPFeed::operator()(const Event<Start> &) {
  connection_.start();
}

void MBPFeed::operator()(const Event<Stop> &) {
  connection_.stop();
}

void MBPFeed::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
}

void MBPFeed::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.ping, metrics::PROFILE)
      .write(profile_.error, metrics::PROFILE)
      .write(profile_.subbed, metrics::PROFILE)
      .write(profile_.mbp, metrics::PROFILE)
      .write(profile_.mbp_snapshot, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void MBPFeed::subscribe(size_t start_from) {
  if (ready())
    subscribe(shared_.symbols.get_slice(index_, start_from));
}

void MBPFeed::operator()(const core::web::ClientSocket::Connected &) {
}

void MBPFeed::operator()(const core::web::ClientSocket::Disconnected &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
}

void MBPFeed::operator()(const core::web::ClientSocket::Ready &) {
  (*this)(ConnectionStatus::READY);
  subscribe();
}

void MBPFeed::operator()(const core::web::ClientSocket::Close &) {
}

void MBPFeed::operator()(const core::web::ClientSocket::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MBPFeed::operator()(const core::web::ClientSocket::Text &) {
  log::fatal("Unexpected"sv);
}

void MBPFeed::operator()(const core::web::ClientSocket::Binary &binary) {
  if (inflate_.decode(binary.payload, inflate_buffer_, [&](auto &payload) {
        std::string_view message{
            reinterpret_cast<char const *>(std::data(payload)), std::size(payload)};
        log::info<5>(R"(message="{}")"sv, message);
        parse(message);
      })) {
  } else {
    log::fatal("Failed to decode message"sv);
  }
}

void MBPFeed::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void MBPFeed::subscribe(const std::span<Symbol const> &symbols) {
  subscribe(symbols, "market"sv, "mbp.20"sv);  // note! 150 is throttled
}

void MBPFeed::subscribe(
    const std::span<Symbol const> &symbols,
    const std::string_view &source,
    const std::string_view &theme) {
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
    log::debug(R"(message="{}")"sv, message);
    connection_.send_text(message);
  }
}

void MBPFeed::request(
    const std::string_view &symbol, const std::string_view &source, const std::string_view &theme) {
  auto id = ++request_id_;
  auto message = fmt::format(
      R"({{)"
      R"("req":"{}.{}.{}",)"
      R"("id":"{}")"
      R"(}})"sv,
      source,
      symbol,
      theme,
      id);
  log::debug(R"(message="{}")"sv, message);
  connection_.send_text(message);
}

void MBPFeed::send_pong(std::chrono::milliseconds timestamp) {
  auto message = fmt::format(
      R"({{)"
      R"("pong":{})"
      R"(}})"sv,
      timestamp.count());
  // log::debug(R"(message="{}")"sv, message);
  connection_.send_text(message);
}

void MBPFeed::parse(const std::string_view &message) {
  profile_.parse([&]() {
    try {
      // log::debug(R"(message="{}")"sv, message);
      auto trace_info = server::create_trace_info();
      core::json::Buffer buffer(decode_buffer_);
      json::Parser::dispatch(*this, message, buffer, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void MBPFeed::operator()(const server::Trace<json::Ping> &event) {
  profile_.ping([&]() {
    auto &[trace_info, ping] = event;
    send_pong(ping.timestamp);
  });
}

void MBPFeed::operator()(const server::Trace<json::Error> &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
  });
}

void MBPFeed::operator()(const server::Trace<json::Subbed> &event) {
  profile_.subbed([&]() {
    auto &[trace_info, subbed] = event;
    log::info<1>("subbed={}"sv, subbed);
  });
}

void MBPFeed::operator()(const server::Trace<json::BBO> &) {
  log::fatal("Unexpected"sv);
}

void MBPFeed::operator()(const server::Trace<json::Trade> &) {
  log::fatal("Unexpected"sv);
}

void MBPFeed::operator()(const server::Trace<json::Detail> &) {
  log::fatal("Unexpected"sv);
}

void MBPFeed::operator()(const server::Trace<json::Ticker> &) {
  log::fatal("Unexpected"sv);
}

void MBPFeed::operator()(const server::Trace<json::MBP> &event) {
  profile_.mbp([&]() {
    // auto &[trace_info, mbp] = event;
    auto &trace_info = event.trace_info;
    auto &mbp = event.value;
    auto symbol = json::extract_symbol(mbp.ch);
    auto &tick = mbp.tick;
    auto &collector = shared_.mbp_collector[symbol];
    core::back_emplacer bids(shared_.bids), asks(shared_.asks);
    for (auto &item : tick.bids)
      bids.emplace_back([&item](auto &result) { emplace(result, item); });
    for (auto &item : tick.asks)
      asks.emplace_back([&item](auto &result) { emplace(result, item); });
    try {
      collector(
          bids,
          asks,
          tick.seq_num,
          tick.seq_num,
          tick.prev_seq_num,
          [&](auto &bids, auto &asks) {  // update
            // log::debug(R"(PUBLISH UPDATE symbol="{}")"sv, symbol);
            MarketByPriceUpdate market_by_price_update{
                .stream_id = stream_id_,
                .exchange = Flags::exchange(),
                .symbol = symbol,
                .bids = bids,
                .asks = asks,
                .update_type = UpdateType::INCREMENTAL,
                .exchange_time_utc = utils::safe_cast(mbp.ts),
                .exchange_sequence = tick.seq_num,
                .price_decimals = {},
                .quantity_decimals = {},
                .checksum = {},
            };
            server::create_trace_and_dispatch(
                handler_, trace_info, market_by_price_update, true, false);
          },
          [&](auto &bids, auto &asks, auto sequence) {  // snapshot
            log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
            MarketByPriceUpdate market_by_price_update{
                .stream_id = stream_id_,
                .exchange = Flags::exchange(),
                .symbol = symbol,
                .bids = bids,
                .asks = asks,
                .update_type = UpdateType::SNAPSHOT,
                .exchange_time_utc = utils::safe_cast(mbp.ts),
                .exchange_sequence = collector.last_sequence(),
                .price_decimals = {},
                .quantity_decimals = {},
                .checksum = {},
            };
            server::Trace event(trace_info, market_by_price_update);
            shared_(event, true, [&](auto &market_by_price) {
              collector.apply(market_by_price, sequence, false);
            });
          },
          [&](auto retries) {  // request
            log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
            request(symbol, "market"sv, "mbp.20"sv);
            /*
            if (retries > Flags::ws_mbp_request_max_retries()) {
              log::fatal("Unexpected"sv);
            }
            auto now = trace_info.source_receive_time;
            shared_.request_queue.emplace_back(now + Flags::ws_mbp_request_delay(), symbol);
            */
          });
    } catch (BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
      // XXX HANS publish stale
      collector.clear();
      auto now = trace_info.source_receive_time;
      // shared_.request_queue.emplace_back(now + Flags::ws_mbp_request_delay(), symbol);
    }
  });
}

void MBPFeed::operator()(const server::Trace<json::MBPSnapshot> &event) {
  profile_.mbp_snapshot([&]() {
    // auto &[trace_info, mbp_snapshot] = event;
    auto &trace_info = event.trace_info;
    auto &mbp_snapshot = event.value;
    auto symbol = json::extract_symbol(mbp_snapshot.rep);
    auto &data = mbp_snapshot.data;
    auto &collector = shared_.mbp_collector[symbol];
    core::back_emplacer bids(shared_.bids), asks(shared_.asks);
    for (auto &item : data.bids)
      bids.emplace_back([&item](auto &result) { emplace(result, item); });
    for (auto &item : data.asks)
      asks.emplace_back([&item](auto &result) { emplace(result, item); });
    try {
      collector(
          bids,
          asks,
          data.seq_num,
          [&](auto &bids, auto &asks, auto sequence) {  // snapshot
            log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
            MarketByPriceUpdate market_by_price_update{
                .stream_id = stream_id_,
                .exchange = Flags::exchange(),
                .symbol = symbol,
                .bids = bids,
                .asks = asks,
                .update_type = UpdateType::SNAPSHOT,
                .exchange_time_utc = {},
                .exchange_sequence = collector.last_sequence(),
                .price_decimals = {},
                .quantity_decimals = {},
                .checksum = {},
            };
            server::Trace event(trace_info, market_by_price_update);
            shared_(event, true, [&](auto &market_by_price) {
              collector.apply(market_by_price, sequence, false);
            });
          },
          [&](auto retries) {  // request
            log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
            /*
            if (retries > Flags::ws_mbp_request_max_retries()) {
              log::fatal("Unexpected"sv);
            }
            auto now = trace_info.source_receive_time;
            shared_.request_queue.emplace_back(now + Flags::ws_mbp_request_delay(), symbol);
            */
            request(symbol, "market"sv, "mbp.20"sv);
          });
    } catch (BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
      // XXX HANS publish stale
      collector.clear();
      auto now = trace_info.source_receive_time;
      // shared_.request_queue.emplace_back(now + Flags::ws_mbp_request_delay(), symbol);
    }
  });
}

}  // namespace huobi
}  // namespace roq
