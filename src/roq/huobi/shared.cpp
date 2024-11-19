/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/huobi/shared.hpp"

namespace roq {
namespace huobi {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : dispatcher_{dispatcher}, settings{settings}, rate_limiter{settings.misc.request_limit, settings.misc.request_limit_interval},
      symbols{settings.ws.max_subscriptions_per_stream} {
}

}  // namespace huobi
}  // namespace roq
