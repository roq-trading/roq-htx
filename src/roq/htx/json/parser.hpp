/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/bbo.hpp"
#include "roq/htx/json/detail.hpp"
#include "roq/htx/json/error.hpp"
#include "roq/htx/json/mbp.hpp"
#include "roq/htx/json/mbp_snapshot.hpp"
#include "roq/htx/json/ping.hpp"
#include "roq/htx/json/subbed.hpp"
#include "roq/htx/json/ticker.hpp"
#include "roq/htx/json/trade.hpp"

namespace roq {
namespace htx {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<Ping> const &) = 0;
    virtual void operator()(Trace<Error> const &) = 0;
    virtual void operator()(Trace<Subbed> const &) = 0;
    virtual void operator()(Trace<BBO> const &) = 0;
    virtual void operator()(Trace<Trade> const &) = 0;
    virtual void operator()(Trace<Detail> const &) = 0;
    virtual void operator()(Trace<Ticker> const &) = 0;
    virtual void operator()(Trace<MBP> const &) = 0;
    virtual void operator()(Trace<MBPSnapshot> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace htx
}  // namespace roq
