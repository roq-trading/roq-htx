/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/htx/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// htx ==> roq

// htx::json::Side ==> roq::Side

template <>
template <>
constexpr Helper<htx::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum htx::json::Side::type_t;
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

static_assert(Helper{htx::json::Side{htx::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{htx::json::Side{htx::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{htx::json::Side{htx::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<htx::json::Side>::helper() const {
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

// roq::Side ==> htx::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<htx::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return htx::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return htx::json::Side::BUY;
    case SELL:
      return htx::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == htx::json::Side{htx::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == htx::json::Side{htx::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == htx::json::Side{htx::json::Side::SELL});

template <>
template <>
std::optional<htx::json::Side> Map<roq::Side>::helper() const {
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
