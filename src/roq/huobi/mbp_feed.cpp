/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/huobi/mbp_feed.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/tools/exception.hpp"

#include "roq/huobi/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace huobi {

// === CONSTANTS ===

namespace {
auto const NAME = "mbp"sv;

auto const SUPPORTS = Mask{
    SupportType::MARKET_BY_PRICE,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.ws.mbp_uri;
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = settings.net.disconnect_on_idle_timeout,
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

MBPFeed::MBPFeed(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared, size_t index)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, index_{index}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_(shared.settings.misc.decode_buffer_size), request_id_{static_cast<uint64_t>(stream_id_) * 1000000},  // scale (debugging)
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
          .total_bytes_received = create_metrics(shared.settings, name_, "total_bytes_received"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .subbed = create_metrics(shared.settings, name_, "subbed"sv),
          .mbp = create_metrics(shared.settings, name_, "mbp"sv),
          .mbp_snapshot = create_metrics(shared.settings, name_, "mbp_snapshot"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      shared_{shared}, inflate_{core::zlib::Inflate::GZIP_NO_HEADER}, request_queue_{shared.settings.ws.request_delay} {
}

void MBPFeed::operator()(Event<Start> const &) {
  (*connection_).start();
}

void MBPFeed::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void MBPFeed::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (ready())
    check_request_queue(now);
}

void MBPFeed::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      .write(counter_.total_bytes_received, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.ping, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.subbed, metrics::Type::PROFILE)
      .write(profile_.mbp, metrics::Type::PROFILE)
      .write(profile_.mbp_snapshot, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void MBPFeed::subscribe(size_t start_from) {
  if (ready())
    subscribe(shared_.symbols.get_slice(index_, start_from));
}

void MBPFeed::operator()(web::socket::Client::Connected const &) {
}

void MBPFeed::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  request_queue_.clear();
}

void MBPFeed::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::READY);
  subscribe();
}

void MBPFeed::operator()(web::socket::Client::Close const &) {
}

void MBPFeed::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MBPFeed::operator()(web::socket::Client::Text const &) {
  log::fatal("Unexpected"sv);
}

void MBPFeed::operator()(web::socket::Client::Binary const &binary) {
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

void MBPFeed::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void MBPFeed::subscribe(std::span<Symbol const> const &symbols) {
  if (std::empty(symbols))
    return;
  subscribe(symbols, "market"sv, "mbp.20"sv);  // note! 150 is throttled
}

void MBPFeed::subscribe(std::span<Symbol const> const &symbols, std::string_view const &source, std::string_view const &theme) {
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

void MBPFeed::request(std::string_view const &symbol, std::string_view const &source, std::string_view const &theme) {
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
  request_queue_.emplace_back(message);
}

void MBPFeed::send_pong(std::chrono::milliseconds timestamp) {
  auto message = fmt::format(
      R"({{)"
      R"("pong":{})"
      R"(}})"sv,
      timestamp.count());
  (*connection_).send_text(message);  // note! special, can't delay
}

void MBPFeed::parse(std::string_view const &message) {
  profile_.parse([&]() {
    try {
      TraceInfo trace_info;
      json::Parser::dispatch(*this, message, decode_buffer_, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void MBPFeed::operator()(Trace<json::Ping> const &event) {
  profile_.ping([&]() {
    auto &[trace_info, ping] = event;
    log::debug("ping={}"sv, ping);
    send_pong(ping.timestamp);
  });
}

void MBPFeed::operator()(Trace<json::Error> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
  });
}

void MBPFeed::operator()(Trace<json::Subbed> const &event) {
  profile_.subbed([&]() {
    auto &[trace_info, subbed] = event;
    log::info<1>("subbed={}"sv, subbed);
  });
}

void MBPFeed::operator()(Trace<json::BBO> const &) {
  log::fatal("Unexpected"sv);
}

void MBPFeed::operator()(Trace<json::Trade> const &) {
  log::fatal("Unexpected"sv);
}

void MBPFeed::operator()(Trace<json::Detail> const &) {
  log::fatal("Unexpected"sv);
}

void MBPFeed::operator()(Trace<json::Ticker> const &) {
  log::fatal("Unexpected"sv);
}

void MBPFeed::operator()(Trace<json::MBP> const &event) {
  profile_.mbp([&]() {
    auto &trace_info = event.trace_info;
    auto &mbp = event.value;
    (*connection_).touch(trace_info.source_receive_time);
    auto symbol = json::extract_symbol(mbp.ch);
    auto &tick = mbp.tick;
    auto &sequencer = shared_.mbp_sequencer[symbol];
    auto &mbp_2 = shared_.get_mbp();
    auto emplace_back = [](auto &result, auto &value) {
      auto mbp_update = MBPUpdate{
          .price = value.price,
          .quantity = value.vol,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
      result.emplace_back(std::move(mbp_update));
    };
    for (auto &item : tick.bids)
      emplace_back(mbp_2.bids, item);
    for (auto &item : tick.asks)
      emplace_back(mbp_2.asks, item);
    try {
      auto create_update = [&](auto &bids, auto &asks, auto update_type, auto exchange_sequence) -> MarketByPriceUpdate {
        return {
            .stream_id = stream_id_,
            .exchange = shared_.settings.exchange,
            .symbol = symbol,
            .bids = bids,
            .asks = asks,
            .update_type = update_type,
            .exchange_time_utc = mbp.ts,
            .exchange_sequence = exchange_sequence,
            .sending_time_utc = {},
            .price_precision = {},
            .quantity_precision = {},
            .checksum = {},
        };
      };
      auto publish_update = [&](auto &bids, auto &asks) {
        // log::debug(R"(PUBLISH UPDATE symbol="{}")"sv, symbol);
        auto market_by_price_update = create_update(bids, asks, UpdateType::INCREMENTAL, tick.seq_num);
        create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
      };
      auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence, [[maybe_unused]] auto retries, [[maybe_unused]] auto delay) {
        log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
        auto market_by_price_update = create_update(bids, asks, UpdateType::SNAPSHOT, sequencer.last_sequence());
        auto apply_updates = [&](auto &market_by_price) { sequencer.apply(market_by_price, sequence, false); };
        Trace event{trace_info, market_by_price_update};
        shared_(event, true, apply_updates);
      };
      auto request_snapshot = [&](auto retries) {
        log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
        if (retries > shared_.settings.ws.mbp_request_max_retries) {
          log::warn(R"(*** EXCEEDED MAX RETRIES: symbol="{}", retries={} ***)"sv, symbol, retries = {});
        } else {
          request(symbol, "market"sv, "mbp.20"sv);
        }
      };
      sequencer(mbp_2.bids, mbp_2.asks, tick.seq_num, tick.seq_num, tick.prev_seq_num, publish_update, publish_snapshot, request_snapshot);
    } catch (BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
      // XXX HANS publish stale
      sequencer.clear();
      request(symbol, "market"sv, "mbp.20"sv);
    }
  });
}

void MBPFeed::operator()(Trace<json::MBPSnapshot> const &event) {
  profile_.mbp_snapshot([&]() {
    auto &trace_info = event.trace_info;
    auto &mbp_snapshot = event.value;
    (*connection_).touch(trace_info.source_receive_time);
    auto symbol = json::extract_symbol(mbp_snapshot.rep);
    auto &data = mbp_snapshot.data;
    auto &sequencer = shared_.mbp_sequencer[symbol];
    auto &mbp = shared_.get_mbp();
    auto emplace_back = [](auto &result, auto &value) {
      auto mbp_update = MBPUpdate{
          .price = value.price,
          .quantity = value.vol,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
      result.emplace_back(std::move(mbp_update));
    };
    for (auto &item : data.bids)
      emplace_back(mbp.bids, item);
    for (auto &item : data.asks)
      emplace_back(mbp.asks, item);
    try {
      auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence, [[maybe_unused]] auto retries, [[maybe_unused]] auto delay) {
        log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
        auto market_by_price_update = MarketByPriceUpdate{
            .stream_id = stream_id_,
            .exchange = shared_.settings.exchange,
            .symbol = symbol,
            .bids = bids,
            .asks = asks,
            .update_type = UpdateType::SNAPSHOT,
            .exchange_time_utc = {},
            .exchange_sequence = sequencer.last_sequence(),
            .sending_time_utc = {},
            .price_precision = {},
            .quantity_precision = {},
            .checksum = {},
        };
        auto apply_updates = [&](auto &market_by_price) { sequencer.apply(market_by_price, sequence, false); };
        Trace event{trace_info, market_by_price_update};
        shared_(event, true, apply_updates);
      };
      auto request_snapshot = [&](auto retries) {
        log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
        if (retries > shared_.settings.ws.mbp_request_max_retries) {
          log::warn(R"(*** EXCEEDED MAX RETRIES: symbol="{}", retries={} ***)"sv, symbol, retries = {});
        } else {
          request(symbol, "market"sv, "mbp.20"sv);
        }
      };
      sequencer(mbp.bids, mbp.asks, data.seq_num, false, publish_snapshot, request_snapshot);
    } catch (BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
      // XXX HANS publish stale
      sequencer.clear();
      request(symbol, "market"sv, "mbp.20"sv);
    }
  });
}

void MBPFeed::check_request_queue(std::chrono::nanoseconds now) {
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
