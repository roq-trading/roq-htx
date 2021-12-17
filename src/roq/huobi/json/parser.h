/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.h"

#include "roq/server.h"

#include "roq/huobi/json/bbo.h"
#include "roq/huobi/json/detail.h"
#include "roq/huobi/json/error.h"
#include "roq/huobi/json/mbp.h"
#include "roq/huobi/json/mbp_snapshot.h"
#include "roq/huobi/json/ping.h"
#include "roq/huobi/json/subbed.h"
#include "roq/huobi/json/ticker.h"
#include "roq/huobi/json/trade.h"

namespace roq {
namespace huobi {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(const server::Trace<Ping> &) = 0;
    virtual void operator()(const server::Trace<Error> &) = 0;
    virtual void operator()(const server::Trace<Subbed> &) = 0;
    virtual void operator()(const server::Trace<BBO> &) = 0;
    virtual void operator()(const server::Trace<Trade> &) = 0;
    virtual void operator()(const server::Trace<Detail> &) = 0;
    virtual void operator()(const server::Trace<Ticker> &) = 0;
    virtual void operator()(const server::Trace<MBP> &) = 0;
    virtual void operator()(const server::Trace<MBPSnapshot> &) = 0;
  };

  static bool dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      const server::TraceInfo &);
};

}  // namespace json
}  // namespace huobi
}  // namespace roq
