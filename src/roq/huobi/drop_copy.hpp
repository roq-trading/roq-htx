/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/core/io/context.hpp"

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
    virtual void operator()(const Trace<StreamStatus const> &) = 0;
    virtual void operator()(const Trace<ExternalLatency const> &) = 0;
    virtual void operator()(const Trace<FundsUpdate const> &, bool is_last) = 0;
  };

  DropCopy(
      Handler &,
      core::io::Context &,
      uint16_t stream_id,
      Security &,
      Shared &,
      const std::string_view &listen_key);

  DropCopy(DropCopy &&) = delete;
  DropCopy(const DropCopy &) = delete;

  bool ready() const;

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(const core::web::ClientSocket::Connected &) override;
  void operator()(const core::web::ClientSocket::Disconnected &) override;
  void operator()(const core::web::ClientSocket::Ready &) override;
  void operator()(const core::web::ClientSocket::Close &) override;
  void operator()(const core::web::ClientSocket::Latency &) override;
  void operator()(const core::web::ClientSocket::Text &) override;
  void operator()(const core::web::ClientSocket::Binary &) override;

 private:
  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void parse(const std::string_view &message);

  void operator()(const Trace<json::Ping const> &) override;
  void operator()(const Trace<json::Error const> &) override;
  void operator()(const Trace<json::Subbed const> &) override;
  void operator()(const Trace<json::BBO const> &) override;
  void operator()(const Trace<json::Trade const> &) override;
  void operator()(const Trace<json::Detail const> &) override;
  void operator()(const Trace<json::Ticker const> &) override;
  void operator()(const Trace<json::MBP const> &) override;
  void operator()(const Trace<json::MBPSnapshot const> &) override;

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
    core::metrics::Profile parse, outbound_account_info, outbound_account_position, balance_update,
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
