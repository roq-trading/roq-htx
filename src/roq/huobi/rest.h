/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_set.h>

#include <string>
#include <string_view>
#include <vector>

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/client.h"

#include "roq/download.h"
#include "roq/server.h"

#include "roq/huobi/rest_state.h"
#include "roq/huobi/shared.h"

#include "roq/huobi/json/currencies.h"
#include "roq/huobi/json/market_status.h"
#include "roq/huobi/json/symbols.h"

namespace roq {
namespace huobi {

class Rest final : public core::web::Client::Handler {
 public:
  struct SymbolsUpdate final {
    std::vector<std::string> &symbols;
  };

  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
    virtual void operator()(const server::Trace<ReferenceData> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<MarketStatus> &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(SymbolsUpdate &) = 0;
  };

  Rest(Handler &, core::io::Context &, uint16_t stream_id, Shared &);

  Rest(Rest &&) = delete;
  Rest(const Rest &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(const core::web::Client::Connected &);
  void operator()(const core::web::Client::Disconnected &);
  void operator()(const core::web::Client::Latency &);

  void operator()(ConnectionStatus);

  uint32_t download(RestState state);

  void get_market_status();
  void get_market_status_ack(const server::Trace<core::web::Response> &, uint32_t sequence);
  void operator()(const server::Trace<json::MarketStatus> &);

  void get_currencies();
  void get_currencies_ack(const server::Trace<core::web::Response> &, uint32_t sequence);
  void operator()(const server::Trace<json::Currencies> &);

  void get_symbols();
  void get_symbols_ack(const server::Trace<core::web::Response> &, uint32_t sequence);
  void operator()(const server::Trace<json::Symbols> &);

  void check_request_queue(std::chrono::nanoseconds now);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  core::web::Client connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile market_status, market_status_ack, currencies, currencies_ack, symbols,
        symbols_ack;
  } profile_;
  struct {
    core::metrics::Latency ping;
  } latency_;
  // cache
  Shared &shared_;
  absl::flat_hash_set<std::string> all_symbols_;
  // state
  bool ready_ = false;
  ConnectionStatus status_ = {};
  server::Download<RestState> download_;
};

}  // namespace huobi
}  // namespace roq
