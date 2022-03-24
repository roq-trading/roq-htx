/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.hpp"

#include "roq/server.hpp"

#include "roq/huobi/json/bbo.hpp"
#include "roq/huobi/json/detail.hpp"
#include "roq/huobi/json/error.hpp"
#include "roq/huobi/json/mbp.hpp"
#include "roq/huobi/json/mbp_snapshot.hpp"
#include "roq/huobi/json/ping.hpp"
#include "roq/huobi/json/subbed.hpp"
#include "roq/huobi/json/ticker.hpp"
#include "roq/huobi/json/trade.hpp"

namespace roq {
namespace huobi {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(const Trace<Ping> &) = 0;
    virtual void operator()(const Trace<Error> &) = 0;
    virtual void operator()(const Trace<Subbed> &) = 0;
    virtual void operator()(const Trace<BBO> &) = 0;
    virtual void operator()(const Trace<Trade> &) = 0;
    virtual void operator()(const Trace<Detail> &) = 0;
    virtual void operator()(const Trace<Ticker> &) = 0;
    virtual void operator()(const Trace<MBP> &) = 0;
    virtual void operator()(const Trace<MBPSnapshot> &) = 0;
  };

  static bool dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      const TraceInfo &);
};

}  // namespace json
}  // namespace huobi
}  // namespace roq
