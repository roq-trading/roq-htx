/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/htx/json/side.hpp"
#include "roq/htx/json/trading.hpp"

#include "roq/side.hpp"
#include "roq/trading_status.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<Side> Map<htx::json::Side>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<htx::json::Trading>::helper() const;

// ===

template <>
template <>
std::optional<htx::json::Side> Map<roq::Side>::helper() const;

}  // namespace roq
