/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/htx/shared.hpp"

namespace roq {
namespace htx {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : api{API::create(settings)}, dispatcher{dispatcher}, settings{settings}, rate_limiter{settings.misc.request_limit, settings.misc.request_limit_interval},
      symbols{settings.ws.max_subscriptions_per_stream} {
}

}  // namespace htx
}  // namespace roq
