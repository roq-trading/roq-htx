/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/htx/protocol/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// htx ==> roq

// htx::protocol::json::Direction ==> roq::Side

template <>
template <>
constexpr Helper<htx::protocol::json::Direction>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum htx::protocol::json::Direction::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Side::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Side::UNDEFINED;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{htx::protocol::json::Direction{htx::protocol::json::Direction::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{htx::protocol::json::Direction{htx::protocol::json::Direction::BUY}} == roq::Side::BUY);
static_assert(Helper{htx::protocol::json::Direction{htx::protocol::json::Direction::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<htx::protocol::json::Direction>::helper() const {
  return Helper{args_};
}

// htx::protocol::json::OrderStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<htx::protocol::json::OrderStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum htx::protocol::json::OrderStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case SUBMITTED:
      return roq::OrderStatus::WORKING;
    case PARTIAL_FILLED:
      return roq::OrderStatus::WORKING;
    case FILLED:
      return roq::OrderStatus::COMPLETED;
    case CANCELED:
      return roq::OrderStatus::CANCELED;
  }
  return {};
}

static_assert(Helper{htx::protocol::json::OrderStatus{htx::protocol::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{htx::protocol::json::OrderStatus{htx::protocol::json::OrderStatus::SUBMITTED}} == roq::OrderStatus::WORKING);
static_assert(Helper{htx::protocol::json::OrderStatus{htx::protocol::json::OrderStatus::PARTIAL_FILLED}} == roq::OrderStatus::WORKING);
static_assert(Helper{htx::protocol::json::OrderStatus{htx::protocol::json::OrderStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{htx::protocol::json::OrderStatus{htx::protocol::json::OrderStatus::CANCELED}} == roq::OrderStatus::CANCELED);

template <>
template <>
std::optional<roq::OrderStatus> Map<htx::protocol::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// htx::protocol::json::OrderType ==> roq::Mask<roq::ExecutionInstruction>

template <>
template <>
constexpr Helper<htx::protocol::json::OrderType>::operator std::optional<roq::Mask<roq::ExecutionInstruction>>() const {
  switch (std::get<0>(args_)) {
    using enum htx::protocol::json::OrderType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Mask<roq::ExecutionInstruction>{};
    case UNKNOWN_INTERNAL:
      return roq::Mask<roq::ExecutionInstruction>{};
    case BUY_MARKET:
      return roq::Mask<roq::ExecutionInstruction>{};
    case SELL_MARKET:
      return roq::Mask<roq::ExecutionInstruction>{};
    case BUY_LIMIT:
      return roq::Mask<roq::ExecutionInstruction>{};
    case SELL_LIMIT:
      return roq::Mask<roq::ExecutionInstruction>{};
    case BUY_IOC:
      return roq::Mask<roq::ExecutionInstruction>{};
    case SELL_IOC:
      return roq::Mask<roq::ExecutionInstruction>{};
    case BUY_LIMIT_MAKER:
      return roq::Mask{roq::ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE};
    case SELL_LIMIT_MAKER:
      return roq::Mask{roq::ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE};
    case BUY_STOP_LIMIT:
      return roq::Mask<roq::ExecutionInstruction>{};
    case SELL_STOP_LIMIT:
      return roq::Mask<roq::ExecutionInstruction>{};
    case BUY_LIMIT_FOK:
      return roq::Mask<roq::ExecutionInstruction>{};
    case SELL_LIMIT_FOK:
      return roq::Mask<roq::ExecutionInstruction>{};
    case BUY_STOP_LIMIT_FOK:
      return roq::Mask<roq::ExecutionInstruction>{};
    case SELL_STOP_LIMIT_FOK:
      return roq::Mask<roq::ExecutionInstruction>{};
  }
  return {};
}

static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::UNDEFINED_INTERNAL}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_MARKET}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_MARKET}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_IOC}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_IOC}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(
    Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT_MAKER}} ==
    roq::Mask{roq::ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE});
static_assert(
    Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT_MAKER}} ==
    roq::Mask{roq::ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_STOP_LIMIT}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_STOP_LIMIT}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT_FOK}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT_FOK}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_STOP_LIMIT_FOK}} == roq::Mask<roq::ExecutionInstruction>{});
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_STOP_LIMIT_FOK}} == roq::Mask<roq::ExecutionInstruction>{});

template <>
template <>
std::optional<roq::Mask<roq::ExecutionInstruction>> Map<htx::protocol::json::OrderType>::helper() const {
  return Helper{args_};
}

// htx::protocol::json::OrderType ==> roq::OrderType

template <>
template <>
constexpr Helper<htx::protocol::json::OrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum htx::protocol::json::OrderType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case BUY_MARKET:
      return roq::OrderType::MARKET;
    case SELL_MARKET:
      return roq::OrderType::MARKET;
    case BUY_LIMIT:
      return roq::OrderType::LIMIT;
    case SELL_LIMIT:
      return roq::OrderType::LIMIT;
    case BUY_IOC:
      return roq::OrderType::LIMIT;
    case SELL_IOC:
      return roq::OrderType::LIMIT;
    case BUY_LIMIT_MAKER:
      return roq::OrderType::LIMIT;
    case SELL_LIMIT_MAKER:
      return roq::OrderType::LIMIT;
    case BUY_STOP_LIMIT:
      return roq::OrderType::LIMIT;
    case SELL_STOP_LIMIT:
      return roq::OrderType::LIMIT;
    case BUY_LIMIT_FOK:
      return roq::OrderType::LIMIT;
    case SELL_LIMIT_FOK:
      return roq::OrderType::LIMIT;
    case BUY_STOP_LIMIT_FOK:
      return roq::OrderType::LIMIT;
    case SELL_STOP_LIMIT_FOK:
      return roq::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_IOC}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_IOC}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT_MAKER}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT_MAKER}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_STOP_LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_STOP_LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT_FOK}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT_FOK}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_STOP_LIMIT_FOK}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_STOP_LIMIT_FOK}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<htx::protocol::json::OrderType>::helper() const {
  return Helper{args_};
}

// htx::protocol::json::OrderType ==> roq::Side

template <>
template <>
constexpr Helper<htx::protocol::json::OrderType>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum htx::protocol::json::OrderType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Side::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Side::UNDEFINED;
    case BUY_MARKET:
      return roq::Side::BUY;
    case SELL_MARKET:
      return roq::Side::SELL;
    case BUY_LIMIT:
      return roq::Side::BUY;
    case SELL_LIMIT:
      return roq::Side::SELL;
    case BUY_IOC:
      return roq::Side::BUY;
    case SELL_IOC:
      return roq::Side::SELL;
    case BUY_LIMIT_MAKER:
      return roq::Side::BUY;
    case SELL_LIMIT_MAKER:
      return roq::Side::SELL;
    case BUY_STOP_LIMIT:
      return roq::Side::BUY;
    case SELL_STOP_LIMIT:
      return roq::Side::SELL;
    case BUY_LIMIT_FOK:
      return roq::Side::BUY;
    case SELL_LIMIT_FOK:
      return roq::Side::SELL;
    case BUY_STOP_LIMIT_FOK:
      return roq::Side::BUY;
    case SELL_STOP_LIMIT_FOK:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_MARKET}} == roq::Side::BUY);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_MARKET}} == roq::Side::SELL);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT}} == roq::Side::BUY);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT}} == roq::Side::SELL);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_IOC}} == roq::Side::BUY);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_IOC}} == roq::Side::SELL);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT_MAKER}} == roq::Side::BUY);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT_MAKER}} == roq::Side::SELL);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_STOP_LIMIT}} == roq::Side::BUY);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_STOP_LIMIT}} == roq::Side::SELL);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT_FOK}} == roq::Side::BUY);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT_FOK}} == roq::Side::SELL);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_STOP_LIMIT_FOK}} == roq::Side::BUY);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_STOP_LIMIT_FOK}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<htx::protocol::json::OrderType>::helper() const {
  return Helper{args_};
}

// htx::protocol::json::OrderType ==> roq::TimeInForce

template <>
template <>
constexpr Helper<htx::protocol::json::OrderType>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum htx::protocol::json::OrderType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case BUY_MARKET:
      return roq::TimeInForce::UNDEFINED;
    case SELL_MARKET:
      return roq::TimeInForce::UNDEFINED;
    case BUY_LIMIT:
      return roq::TimeInForce::GTC;
    case SELL_LIMIT:
      return roq::TimeInForce::GTC;
    case BUY_IOC:
      return roq::TimeInForce::IOC;
    case SELL_IOC:
      return roq::TimeInForce::IOC;
    case BUY_LIMIT_MAKER:
      return roq::TimeInForce::GTC;
    case SELL_LIMIT_MAKER:
      return roq::TimeInForce::GTC;
    case BUY_STOP_LIMIT:
      return roq::TimeInForce::GTC;
    case SELL_STOP_LIMIT:
      return roq::TimeInForce::GTC;
    case BUY_LIMIT_FOK:
      return roq::TimeInForce::FOK;
    case SELL_LIMIT_FOK:
      return roq::TimeInForce::FOK;
    case BUY_STOP_LIMIT_FOK:
      return roq::TimeInForce::FOK;
    case SELL_STOP_LIMIT_FOK:
      return roq::TimeInForce::FOK;
  }
  return {};
}

static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_MARKET}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_MARKET}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT}} == roq::TimeInForce::GTC);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT}} == roq::TimeInForce::GTC);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_IOC}} == roq::TimeInForce::IOC);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_IOC}} == roq::TimeInForce::IOC);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT_MAKER}} == roq::TimeInForce::GTC);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT_MAKER}} == roq::TimeInForce::GTC);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_STOP_LIMIT}} == roq::TimeInForce::GTC);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_STOP_LIMIT}} == roq::TimeInForce::GTC);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT_FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT_FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_STOP_LIMIT_FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_STOP_LIMIT_FOK}} == roq::TimeInForce::FOK);

template <>
template <>
std::optional<roq::TimeInForce> Map<htx::protocol::json::OrderType>::helper() const {
  return Helper{args_};
}

// htx::protocol::json::Trading ==> roq::TradingStatus

template <>
template <>
constexpr Helper<htx::protocol::json::Trading>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum htx::protocol::json::Trading::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
    case ENABLED:
      return roq::TradingStatus::OPEN;
    case DISABLED:
      return roq::TradingStatus::CLOSE;
  }
  return {};
}

static_assert(Helper{htx::protocol::json::Trading{htx::protocol::json::Trading::UNDEFINED_INTERNAL}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{htx::protocol::json::Trading{htx::protocol::json::Trading::ENABLED}} == roq::TradingStatus::OPEN);
static_assert(Helper{htx::protocol::json::Trading{htx::protocol::json::Trading::DISABLED}} == roq::TradingStatus::CLOSE);

template <>
template <>
std::optional<roq::TradingStatus> Map<htx::protocol::json::Trading>::helper() const {
  return Helper{args_};
}

// roq ==> htx::json

// roq::Side ==> htx::protocol::json::Direction

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<htx::protocol::json::Direction>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return htx::protocol::json::Direction::UNDEFINED_INTERNAL;
    case BUY:
      return htx::protocol::json::Direction::BUY;
    case SELL:
      return htx::protocol::json::Direction::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == htx::protocol::json::Direction{htx::protocol::json::Direction::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == htx::protocol::json::Direction{htx::protocol::json::Direction::BUY});
static_assert(Helper{roq::Side::SELL} == htx::protocol::json::Direction{htx::protocol::json::Direction::SELL});

template <>
template <>
std::optional<htx::protocol::json::Direction> Map<roq::Side>::helper() const {
  return Helper{args_};
}

// {roq::Side, roq::OrderType} ==> htx::protocol::json::OrderType

template <>
template <>
constexpr Helper<roq::Side, roq::OrderType, roq::TimeInForce, roq::Mask<roq::ExecutionInstruction>>::operator std::optional<htx::protocol::json::OrderType>()
    const {
  auto &[side, order_type, time_in_force, execution_instructions] = args_;
  auto maker = [&]() { return execution_instructions.has(ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE); }();
  switch (side) {
    using enum roq::Side;
    case UNDEFINED:
      return htx::protocol::json::OrderType::UNDEFINED_INTERNAL;
    case BUY: {
      if (time_in_force == TimeInForce::IOC) {
        return htx::protocol::json::OrderType::BUY_IOC;
      }
      switch (order_type) {
        using enum roq::OrderType;
        case UNDEFINED:
          return htx::protocol::json::OrderType::UNDEFINED_INTERNAL;
        case MARKET:
          return htx::protocol::json::OrderType::BUY_MARKET;
        case LIMIT:
          if (maker) {
            return htx::protocol::json::OrderType::BUY_LIMIT_MAKER;
          } else if (time_in_force == TimeInForce::FOK) {
            return htx::protocol::json::OrderType::BUY_LIMIT_FOK;
          } else {
            return htx::protocol::json::OrderType::BUY_LIMIT;
          }
      }
      break;
    }
    case SELL: {
      if (time_in_force == TimeInForce::IOC) {
        return htx::protocol::json::OrderType::SELL_IOC;
      }
      switch (order_type) {
        using enum roq::OrderType;
        case UNDEFINED:
          return htx::protocol::json::OrderType::UNDEFINED_INTERNAL;
        case MARKET:
          return htx::protocol::json::OrderType::SELL_MARKET;
        case LIMIT:
          if (maker) {
            return htx::protocol::json::OrderType::SELL_LIMIT_MAKER;
          } else if (time_in_force == TimeInForce::FOK) {
            return htx::protocol::json::OrderType::SELL_LIMIT_FOK;
          } else {
            return htx::protocol::json::OrderType::SELL_LIMIT;
          }
      }
      break;
    }
  }
  return {};
}

static_assert(
    Helper{roq::Side::UNDEFINED, roq::OrderType::UNDEFINED, roq::TimeInForce::UNDEFINED, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::Side::BUY, roq::OrderType::MARKET, roq::TimeInForce::UNDEFINED, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_MARKET});
static_assert(
    Helper{roq::Side::SELL, roq::OrderType::MARKET, roq::TimeInForce::UNDEFINED, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_MARKET});
static_assert(
    Helper{roq::Side::BUY, roq::OrderType::LIMIT, roq::TimeInForce::UNDEFINED, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT});
static_assert(
    Helper{roq::Side::SELL, roq::OrderType::LIMIT, roq::TimeInForce::UNDEFINED, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT});
static_assert(
    Helper{roq::Side::BUY, roq::OrderType::MARKET, roq::TimeInForce::IOC, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_IOC});
static_assert(
    Helper{roq::Side::SELL, roq::OrderType::MARKET, roq::TimeInForce::IOC, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_IOC});
static_assert(
    Helper{roq::Side::BUY, roq::OrderType::LIMIT, roq::TimeInForce::IOC, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_IOC});
static_assert(
    Helper{roq::Side::SELL, roq::OrderType::LIMIT, roq::TimeInForce::IOC, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_IOC});
static_assert(
    Helper{roq::Side::BUY, roq::OrderType::LIMIT, roq::TimeInForce::FOK, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT_FOK});
static_assert(
    Helper{roq::Side::SELL, roq::OrderType::LIMIT, roq::TimeInForce::FOK, roq::Mask<roq::ExecutionInstruction>{}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT_FOK});
static_assert(
    Helper{roq::Side::BUY, roq::OrderType::LIMIT, roq::TimeInForce::FOK, roq::Mask{roq::ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::BUY_LIMIT_MAKER});
static_assert(
    Helper{roq::Side::SELL, roq::OrderType::LIMIT, roq::TimeInForce::FOK, roq::Mask{roq::ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE}} ==
    htx::protocol::json::OrderType{htx::protocol::json::OrderType::SELL_LIMIT_MAKER});

template <>
template <>
std::optional<htx::protocol::json::OrderType> Map<roq::Side, roq::OrderType, roq::TimeInForce, roq::Mask<roq::ExecutionInstruction>>::helper() const {
  return Helper{args_};
}

}  // namespace roq
