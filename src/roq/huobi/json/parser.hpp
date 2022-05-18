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
    virtual void operator()(Trace<Ping const> const &) = 0;
    virtual void operator()(Trace<Error const> const &) = 0;
    virtual void operator()(Trace<Subbed const> const &) = 0;
    virtual void operator()(Trace<BBO const> const &) = 0;
    virtual void operator()(Trace<Trade const> const &) = 0;
    virtual void operator()(Trace<Detail const> const &) = 0;
    virtual void operator()(Trace<Ticker const> const &) = 0;
    virtual void operator()(Trace<MBP const> const &) = 0;
    virtual void operator()(Trace<MBPSnapshot const> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::Buffer &, TraceInfo const &);
};

}  // namespace json
}  // namespace huobi
}  // namespace roq
