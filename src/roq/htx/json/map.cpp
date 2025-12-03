/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/htx/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// htx ==> roq

// htx::json::Direction ==> roq::Side

template <>
template <>
constexpr Helper<htx::json::Direction>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum htx::json::Direction::type_t;
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

static_assert(Helper{htx::json::Direction{htx::json::Direction::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{htx::json::Direction{htx::json::Direction::BUY}} == roq::Side::BUY);
static_assert(Helper{htx::json::Direction{htx::json::Direction::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<htx::json::Direction>::helper() const {
  return Helper{args_};
}

// htx::json::OrderStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<htx::json::OrderStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum htx::json::OrderStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case SUBMITTED:
      return roq::OrderStatus::WORKING;
    case CANCELED:
      return roq::OrderStatus::CANCELED;
  }
  return {};
}

static_assert(Helper{htx::json::OrderStatus{htx::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{htx::json::OrderStatus{htx::json::OrderStatus::SUBMITTED}} == roq::OrderStatus::WORKING);
static_assert(Helper{htx::json::OrderStatus{htx::json::OrderStatus::CANCELED}} == roq::OrderStatus::CANCELED);

template <>
template <>
std::optional<roq::OrderStatus> Map<htx::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// htx::json::OrderType ==> roq::OrderType

template <>
template <>
constexpr Helper<htx::json::OrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum htx::json::OrderType::type_t;
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

static_assert(Helper{htx::json::OrderType{htx::json::OrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_IOC}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_IOC}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_LIMIT_MAKER}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_LIMIT_MAKER}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_STOP_LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_STOP_LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_LIMIT_FOK}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_LIMIT_FOK}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_STOP_LIMIT_FOK}} == roq::OrderType::LIMIT);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_STOP_LIMIT_FOK}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<htx::json::OrderType>::helper() const {
  return Helper{args_};
}

// htx::json::OrderType ==> roq::Side

template <>
template <>
constexpr Helper<htx::json::OrderType>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum htx::json::OrderType::type_t;
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

static_assert(Helper{htx::json::OrderType{htx::json::OrderType::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_MARKET}} == roq::Side::BUY);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_MARKET}} == roq::Side::SELL);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_LIMIT}} == roq::Side::BUY);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_LIMIT}} == roq::Side::SELL);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_IOC}} == roq::Side::BUY);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_IOC}} == roq::Side::SELL);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_LIMIT_MAKER}} == roq::Side::BUY);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_LIMIT_MAKER}} == roq::Side::SELL);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_STOP_LIMIT}} == roq::Side::BUY);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_STOP_LIMIT}} == roq::Side::SELL);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_LIMIT_FOK}} == roq::Side::BUY);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_LIMIT_FOK}} == roq::Side::SELL);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::BUY_STOP_LIMIT_FOK}} == roq::Side::BUY);
static_assert(Helper{htx::json::OrderType{htx::json::OrderType::SELL_STOP_LIMIT_FOK}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<htx::json::OrderType>::helper() const {
  return Helper{args_};
}

// htx::json::Trading ==> roq::TradingStatus

template <>
template <>
constexpr Helper<htx::json::Trading>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum htx::json::Trading::type_t;
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

static_assert(Helper{htx::json::Trading{htx::json::Trading::UNDEFINED_INTERNAL}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{htx::json::Trading{htx::json::Trading::ENABLED}} == roq::TradingStatus::OPEN);
static_assert(Helper{htx::json::Trading{htx::json::Trading::DISABLED}} == roq::TradingStatus::CLOSE);

template <>
template <>
std::optional<roq::TradingStatus> Map<htx::json::Trading>::helper() const {
  return Helper{args_};
}

// roq ==> htx::json

// roq::Side ==> htx::json::Direction

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<htx::json::Direction>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return htx::json::Direction::UNDEFINED_INTERNAL;
    case BUY:
      return htx::json::Direction::BUY;
    case SELL:
      return htx::json::Direction::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == htx::json::Direction{htx::json::Direction::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == htx::json::Direction{htx::json::Direction::BUY});
static_assert(Helper{roq::Side::SELL} == htx::json::Direction{htx::json::Direction::SELL});

template <>
template <>
std::optional<htx::json::Direction> Map<roq::Side>::helper() const {
  return Helper{args_};
}

// {roq::Side, roq::OrderType} ==> htx::json::OrderType

template <>
template <>
constexpr Helper<roq::Side, roq::OrderType>::operator std::optional<htx::json::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return htx::json::OrderType::UNDEFINED_INTERNAL;
    case BUY:
      switch (std::get<1>(args_)) {
        using enum roq::OrderType;
        case UNDEFINED:
          return htx::json::OrderType::UNDEFINED_INTERNAL;
        case MARKET:
          return htx::json::OrderType::BUY_MARKET;
        case LIMIT:
          return htx::json::OrderType::BUY_LIMIT;
      }
      break;
    case SELL:
      switch (std::get<1>(args_)) {
        using enum roq::OrderType;
        case UNDEFINED:
          return htx::json::OrderType::UNDEFINED_INTERNAL;
        case MARKET:
          return htx::json::OrderType::SELL_MARKET;
        case LIMIT:
          return htx::json::OrderType::SELL_LIMIT;
      }
      break;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED, roq::OrderType::UNDEFINED} == htx::json::OrderType{htx::json::OrderType::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY, roq::OrderType::MARKET} == htx::json::OrderType{htx::json::OrderType::BUY_MARKET});
static_assert(Helper{roq::Side::SELL, roq::OrderType::MARKET} == htx::json::OrderType{htx::json::OrderType::SELL_MARKET});
static_assert(Helper{roq::Side::BUY, roq::OrderType::LIMIT} == htx::json::OrderType{htx::json::OrderType::BUY_LIMIT});
static_assert(Helper{roq::Side::SELL, roq::OrderType::LIMIT} == htx::json::OrderType{htx::json::OrderType::SELL_LIMIT});

template <>
template <>
std::optional<htx::json::OrderType> Map<roq::Side, roq::OrderType>::helper() const {
  return Helper{args_};
}

}  // namespace roq
