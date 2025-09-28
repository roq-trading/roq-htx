/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/htx/json/map.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace htx {
namespace json {

// === HELPERS ===

namespace {
// note! constexpr helper for static testing
template <typename... Args>
struct Helper final {
  explicit constexpr Helper(std::tuple<Args...> const &args) : args_{args} {}
  explicit constexpr Helper(Args &&...args_) : args_{std::forward<Args>(args_)...} {}

  template <typename R>
  constexpr operator R();

 private:
  std::tuple<Args...> const args_;
};

// ==> roq

// Side ==> roq::Side

template <>
template <>
constexpr Helper<Side>::operator roq::Side() {
  switch (std::get<0>(args_)) {
    using enum Side::type_t;
    case UNDEFINED_INTERNAL:
      return {};
    case UNKNOWN_INTERNAL:
      break;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::Side>(Helper{Side{Side::UNDEFINED_INTERNAL}}) == roq::Side::UNDEFINED);
static_assert(static_cast<roq::Side>(Helper{Side{Side::BUY}}) == roq::Side::BUY);
static_assert(static_cast<roq::Side>(Helper{Side{Side::SELL}}) == roq::Side::SELL);

// Trading ==> roq::Side

template <>
template <>
constexpr Helper<Trading>::operator roq::TradingStatus() {
  switch (std::get<0>(args_)) {
    using enum json::Trading::type_t;
    case UNDEFINED_INTERNAL:
      return {};
    case UNKNOWN_INTERNAL:
      break;
    case ENABLED:
      return roq::TradingStatus::OPEN;
    case DISABLED:
      return roq::TradingStatus::HALT;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::TradingStatus>(Helper{Trading{Trading::UNDEFINED_INTERNAL}}) == roq::TradingStatus::UNDEFINED);
static_assert(static_cast<roq::TradingStatus>(Helper{Trading{Trading::ENABLED}}) == roq::TradingStatus::OPEN);
static_assert(static_cast<roq::TradingStatus>(Helper{Trading{Trading::DISABLED}}) == roq::TradingStatus::HALT);

// roq ==>

// Side ==> roq::Side

template <>
template <>
constexpr Helper<roq::Side>::operator Side() {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return {};
    case BUY:
      return json::Side::BUY;
    case SELL:
      return json::Side::SELL;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<Side>(Helper{roq::Side::UNDEFINED}) == Side{Side::UNDEFINED_INTERNAL});
static_assert(static_cast<Side>(Helper{roq::Side::BUY}) == Side{Side::BUY});
static_assert(static_cast<Side>(Helper{roq::Side::SELL}) == Side{Side::SELL});
}  // namespace

// === IMPLEMENTATION ===

// ==> roq

template <>
template <>
Map<Side>::operator roq::Side() {
  return Helper{args_};
}

template <>
template <>
Map<Trading>::operator roq::TradingStatus() {
  return Helper{args_};
}

template <>
template <>
Map<roq::Side>::operator Side() {
  return Helper{args_};
}

}  // namespace json
}  // namespace htx
}  // namespace roq
