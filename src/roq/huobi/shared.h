/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <deque>
#include <string>
#include <utility>

#include "roq/api.h"
#include "roq/server.h"

#include "roq/core/memory.h"

#include "roq/core/limit/rate_limiter.h"

namespace roq {
namespace huobi {

struct Shared final {
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(const Shared &) = delete;

  auto discard_symbol(const std::string_view &name) const {
    return dispatcher_.discard_symbol(name);
  }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

  template <typename F>
  bool can_request(std::chrono::nanoseconds now, F callback) {
    return rate_limiter_.can_request(now, callback);
  }

 public:
  core::page_aligned_vector<MBPUpdate> bids, asks, final_bids, final_asks;

  std::deque<std::pair<std::chrono::nanoseconds, std::string> > request_queue;

 private:
  server::Dispatcher &dispatcher_;

  core::limit::RateLimiter rate_limiter_;
};

}  // namespace huobi
}  // namespace roq
