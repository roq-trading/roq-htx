/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <vector>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/zlib/inflate.hpp"

#include "roq/core/timer_queue.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/htx/gateway/shared.hpp"

#include "roq/htx/protocol/json/parser.hpp"

namespace roq {
namespace htx {
namespace gateway {

struct MarketData final : public Base<MarketData>,
                          public server::MarketDataStream,
                          public web::socket::Client::Handler,
                          public protocol::json::Parser::Handler {
  struct Handler {};

  MarketData(Handler &, io::Context &, uint16_t stream_id, Shared &, size_t index);

  // protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

  // server::MarketDataStream

  void subscribe(size_t start_from = 0) override;

 protected:
  // web::socket::Client::Handler

  void operator()(Trace<web::socket::Connected> const &) override;
  void operator()(Trace<web::socket::Disconnected> const &) override;
  void operator()(Trace<web::socket::Ready> const &) override;
  void operator()(Trace<web::socket::Close> const &) override;
  void operator()(Trace<web::socket::Latency> const &) override;
  void operator()(Trace<web::socket::Text> const &) override;
  void operator()(Trace<web::socket::Binary> const &) override;

  // protocol::json::Parser::Handler

  void operator()(Trace<protocol::json::Req> const &) override;
  void operator()(Trace<protocol::json::Ping> const &) override;
  void operator()(Trace<protocol::json::Ping2> const &) override;
  void operator()(Trace<protocol::json::Error> const &) override;
  void operator()(Trace<protocol::json::Error2> const &) override;
  void operator()(Trace<protocol::json::Sub> const &) override;
  void operator()(Trace<protocol::json::Subbed> const &) override;
  void operator()(Trace<protocol::json::BBO> const &) override;
  void operator()(Trace<protocol::json::Trade> const &) override;
  void operator()(Trace<protocol::json::Detail> const &) override;
  void operator()(Trace<protocol::json::Ticker> const &) override;
  void operator()(Trace<protocol::json::MBP> const &) override;
  void operator()(Trace<protocol::json::MBPSnapshot> const &) override;
  //
  void operator()(Trace<protocol::json::Accounts> const &) override;
  void operator()(Trace<protocol::json::Orders> const &) override;
  void operator()(Trace<protocol::json::Clearing> const &) override;

  // helpers

  void check_request_queue(std::chrono::nanoseconds now);

  // helpers

  void subscribe(std::span<Symbol const> const &symbols);

  void subscribe(std::string_view const &, std::string_view const &source, std::string_view const &theme);

  void send_pong(std::chrono::milliseconds timestamp);

  void parse(std::string_view const &message);

 private:
  [[maybe_unused]] Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  size_t const index_;
  // web socket
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // session
  uint64_t request_id_ = {};
  // metrics
  struct {
    utils::metrics::Counter disconnect, total_bytes_received;
  } counter_;
  struct {
    utils::metrics::Profile parse, ping, error, subbed, bbo, trade, detail, ticker;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  // zlib
  core::zlib::Inflate inflate_;
  std::vector<std::byte> inflate_buffer_;
  // queue
  core::TimerQueue<std::string> request_queue_;
};

}  // namespace gateway
}  // namespace htx
}  // namespace roq
