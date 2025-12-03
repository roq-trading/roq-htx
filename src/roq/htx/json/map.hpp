/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/htx/json/direction.hpp"
#include "roq/htx/json/order_status.hpp"
#include "roq/htx/json/order_type.hpp"
#include "roq/htx/json/trading.hpp"

#include "roq/order_status.hpp"
#include "roq/order_type.hpp"
#include "roq/side.hpp"
#include "roq/trading_status.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<Side> Map<htx::json::Direction>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<htx::json::OrderStatus>::helper() const;

template <>
template <>
std::optional<OrderType> Map<htx::json::OrderType>::helper() const;

template <>
template <>
std::optional<Side> Map<htx::json::OrderType>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<htx::json::Trading>::helper() const;

// ===

template <>
template <>
std::optional<htx::json::Direction> Map<roq::Side>::helper() const;

template <>
template <>
std::optional<htx::json::OrderType> Map<roq::Side, roq::OrderType>::helper() const;

}  // namespace roq
