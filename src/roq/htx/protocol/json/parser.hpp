/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/protocol/json/bbo.hpp"
#include "roq/htx/protocol/json/detail.hpp"
#include "roq/htx/protocol/json/error.hpp"
#include "roq/htx/protocol/json/error_2.hpp"
#include "roq/htx/protocol/json/mbp.hpp"
#include "roq/htx/protocol/json/mbp_snapshot.hpp"
#include "roq/htx/protocol/json/ping.hpp"
#include "roq/htx/protocol/json/ping_2.hpp"
#include "roq/htx/protocol/json/req.hpp"
#include "roq/htx/protocol/json/sub.hpp"
#include "roq/htx/protocol/json/subbed.hpp"
#include "roq/htx/protocol/json/ticker.hpp"
#include "roq/htx/protocol/json/trade.hpp"

#include "roq/htx/protocol/json/accounts.hpp"
#include "roq/htx/protocol/json/clearing.hpp"
#include "roq/htx/protocol/json/orders.hpp"

namespace roq {
namespace htx {
namespace protocol {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<Req> const &) = 0;   // dc
    virtual void operator()(Trace<Ping> const &) = 0;  // dc
    virtual void operator()(Trace<Ping2> const &) = 0;
    virtual void operator()(Trace<Error> const &) = 0;
    virtual void operator()(Trace<Error2> const &) = 0;
    virtual void operator()(Trace<Sub> const &) = 0;  // dc
    virtual void operator()(Trace<Subbed> const &) = 0;
    virtual void operator()(Trace<BBO> const &) = 0;
    virtual void operator()(Trace<Trade> const &) = 0;
    virtual void operator()(Trace<Detail> const &) = 0;
    virtual void operator()(Trace<Ticker> const &) = 0;
    virtual void operator()(Trace<MBP> const &) = 0;
    virtual void operator()(Trace<MBPSnapshot> const &) = 0;
    virtual void operator()(Trace<Accounts> const &) = 0;
    virtual void operator()(Trace<Orders> const &) = 0;
    virtual void operator()(Trace<Clearing> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace protocol
}  // namespace htx
}  // namespace roq
