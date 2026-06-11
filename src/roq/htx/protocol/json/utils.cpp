/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/htx/protocol/json/utils.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace htx {
namespace protocol {
namespace json {

// === HELPERS ===

namespace {
constexpr std::string_view extract_symbol_helper(std::string_view const &channel) {
  auto sep1 = channel.find_first_of('.');
  if (sep1 != channel.npos) {
    ++sep1;
    auto sep2 = channel.find_first_of('.', sep1);
    if (sep2 != channel.npos) {
      return channel.substr(sep1, sep2 - sep1);
    }
    return channel.substr(sep1);
  }
  return channel;
}

static_assert(extract_symbol_helper("market.btcusdt.ticker"sv) == "btcusdt"sv);
static_assert(extract_symbol_helper("market.btcusdt.bbo"sv) == "btcusdt"sv);
static_assert(extract_symbol_helper("market.btcusdt.trade.detail"sv) == "btcusdt"sv);
static_assert(extract_symbol_helper("market.btcusdt.detail"sv) == "btcusdt"sv);
static_assert(extract_symbol_helper("market.btcusdt.mbp.20"sv) == "btcusdt"sv);
}  // namespace

// === IMPLEMENTATION ===

Error guess_error([[maybe_unused]] int32_t err_code) {
  return {};
}

Error guess_error([[maybe_unused]] std::string_view const &message) {
  return {};
}

std::string_view extract_symbol(std::string_view const &channel) {
  return extract_symbol_helper(channel);
}

}  // namespace json
}  // namespace protocol
}  // namespace htx
}  // namespace roq
