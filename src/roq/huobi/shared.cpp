/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/huobi/shared.hpp"

#include "roq/huobi/flags.hpp"

namespace roq {
namespace huobi {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher)
    : dispatcher_{dispatcher}, rate_limiter{Flags::request_limit(), Flags::request_limit_interval()},
      symbols{Flags::ws_max_subscriptions_per_stream()} {
}

}  // namespace huobi
}  // namespace roq
