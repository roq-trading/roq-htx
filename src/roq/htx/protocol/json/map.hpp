/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/htx/protocol/json/direction.hpp"
#include "roq/htx/protocol/json/order_status.hpp"
#include "roq/htx/protocol/json/order_type.hpp"
#include "roq/htx/protocol/json/trading.hpp"

#include "roq/execution_instruction.hpp"
#include "roq/order_status.hpp"
#include "roq/order_type.hpp"
#include "roq/side.hpp"
#include "roq/time_in_force.hpp"
#include "roq/trading_status.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<Side> Map<htx::protocol::json::Direction>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<htx::protocol::json::OrderStatus>::helper() const;

template <>
template <>
std::optional<Mask<ExecutionInstruction>> Map<htx::protocol::json::OrderType>::helper() const;

template <>
template <>
std::optional<OrderType> Map<htx::protocol::json::OrderType>::helper() const;

template <>
template <>
std::optional<Side> Map<htx::protocol::json::OrderType>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<htx::protocol::json::OrderType>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<htx::protocol::json::Trading>::helper() const;

// ===

template <>
template <>
std::optional<htx::protocol::json::Direction> Map<Side>::helper() const;

template <>
template <>
std::optional<htx::protocol::json::OrderType> Map<Side, OrderType, TimeInForce, Mask<ExecutionInstruction>>::helper() const;

}  // namespace roq
