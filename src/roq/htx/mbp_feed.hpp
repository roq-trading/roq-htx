/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/zlib/inflate.hpp"

#include "roq/core/download.hpp"
#include "roq/core/timer_queue.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/htx/shared.hpp"

#include "roq/htx/json/parser.hpp"

namespace roq {
namespace htx {

struct MBPFeed final : public web::socket::Client::Handler, public json::Parser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) = 0;
  };

  MBPFeed(Handler &, io::Context &, uint16_t stream_id, Shared &, size_t index);

  MBPFeed(MBPFeed const &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  void subscribe(size_t start_from = 0);

 protected:
  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  void subscribe(std::span<Symbol const> const &symbols);

  void subscribe(std::string_view const &, std::string_view const &source, std::string_view const &theme);

  void request(std::string_view const &symbol, std::string_view const &source, std::string_view const &theme);

  void send_pong(std::chrono::milliseconds timestamp);

  void parse(std::string_view const &message);

  void operator()(Trace<json::Req> const &) override;
  void operator()(Trace<json::Ping> const &) override;
  void operator()(Trace<json::Ping2> const &) override;
  void operator()(Trace<json::Error> const &) override;
  void operator()(Trace<json::Error2> const &) override;
  void operator()(Trace<json::Sub> const &) override;
  void operator()(Trace<json::Subbed> const &) override;
  void operator()(Trace<json::BBO> const &) override;
  void operator()(Trace<json::Trade> const &) override;
  void operator()(Trace<json::Detail> const &) override;
  void operator()(Trace<json::Ticker> const &) override;
  void operator()(Trace<json::MBP> const &) override;
  void operator()(Trace<json::MBPSnapshot> const &) override;
  //
  void operator()(Trace<json::Accounts> const &) override;
  void operator()(Trace<json::Orders> const &) override;

  void check_request_queue(std::chrono::nanoseconds now);

 private:
  Handler &handler_;
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
    utils::metrics::Profile parse, ping, error, subbed, mbp, mbp_snapshot;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus status_ = {};
  // zlib
  core::zlib::Inflate inflate_;
  std::vector<std::byte> inflate_buffer_;
  // queue
  core::TimerQueue<std::string> request_queue_;
};

}  // namespace htx
}  // namespace roq
