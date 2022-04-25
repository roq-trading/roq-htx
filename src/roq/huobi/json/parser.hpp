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
    virtual void operator()(const Trace<Ping const> &) = 0;
    virtual void operator()(const Trace<Error const> &) = 0;
    virtual void operator()(const Trace<Subbed const> &) = 0;
    virtual void operator()(const Trace<BBO const> &) = 0;
    virtual void operator()(const Trace<Trade const> &) = 0;
    virtual void operator()(const Trace<Detail const> &) = 0;
    virtual void operator()(const Trace<Ticker const> &) = 0;
    virtual void operator()(const Trace<MBP const> &) = 0;
    virtual void operator()(const Trace<MBPSnapshot const> &) = 0;
  };

  static bool dispatch(
      Handler &, const std::string_view &message, core::json::Buffer &, const TraceInfo &);
};

}  // namespace json
}  // namespace huobi
}  // namespace roq
