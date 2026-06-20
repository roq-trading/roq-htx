/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/htx/gateway/account.hpp"
#include "roq/htx/gateway/shared.hpp"

#include "roq/htx/protocol/json/parser.hpp"

namespace roq {
namespace htx {
namespace gateway {

struct DropCopy final : public web::socket::Client::Handler, public protocol::json::Parser::Handler {
  struct Handler {};

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  DropCopy(DropCopy const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

 protected:
  // web::socket::Client::Handler

  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

  // helpers

  bool ready() const;

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  void send_pong(std::chrono::milliseconds timestamp);

  void send_login();

  void subscribe();
  void subscribe(std::string_view const &channel);

  void parse(std::string_view const &message);

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

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // web socket
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse,  //
        req, ping, error, sub, outbound_account_info, outbound_account_position, balance_update, execution_report;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  bool ready_ = false;
  ConnectionStatus connection_status_ = {};
};

}  // namespace gateway
}  // namespace htx
}  // namespace roq
