/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/core/web/client_socket.hpp"

#include "roq/server.hpp"

#include "roq/huobi/drop_copy_state.hpp"
#include "roq/huobi/security.hpp"
#include "roq/huobi/shared.hpp"

#include "roq/huobi/json/parser.hpp"

namespace roq {
namespace huobi {

class DropCopy final : public core::web::ClientSocket::Handler, public json::Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<FundsUpdate const> const &, bool is_last) = 0;
  };

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Security &, Shared &, std::string_view const &listen_key);

  DropCopy(DropCopy &&) = delete;
  DropCopy(DropCopy const &) = delete;

  bool ready() const;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(core::web::ClientSocket::Connected const &) override;
  void operator()(core::web::ClientSocket::Disconnected const &) override;
  void operator()(core::web::ClientSocket::Ready const &) override;
  void operator()(core::web::ClientSocket::Close const &) override;
  void operator()(core::web::ClientSocket::Latency const &) override;
  void operator()(core::web::ClientSocket::Text const &) override;
  void operator()(core::web::ClientSocket::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void parse(std::string_view const &message);

  void operator()(Trace<json::Ping const> const &) override;
  void operator()(Trace<json::Error const> const &) override;
  void operator()(Trace<json::Subbed const> const &) override;
  void operator()(Trace<json::BBO const> const &) override;
  void operator()(Trace<json::Trade const> const &) override;
  void operator()(Trace<json::Detail const> const &) override;
  void operator()(Trace<json::Ticker const> const &) override;
  void operator()(Trace<json::MBP const> const &) override;
  void operator()(Trace<json::MBPSnapshot const> const &) override;

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // web socket
  core::web::ClientSocket connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, ping, error, outbound_account_info, outbound_account_position, balance_update,
        execution_report;
  } profile_;
  struct {
    core::metrics::Latency ping, heartbeat;
  } latency_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  // state
  // state
  bool ready_ = false;
  ConnectionStatus status_ = {};
  core::Download<DropCopyState> download_;
};

}  // namespace huobi
}  // namespace roq
