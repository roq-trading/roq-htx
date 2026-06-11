/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/htx/protocol/json/parser.hpp"

#include <algorithm>
#include <cctype>
#include <string>

#include "roq/compat.hpp"

#include "roq/logging.hpp"

#include "roq/utils/hash/fnv.hpp"

#include "roq/htx/protocol/json/topic.hpp"
#include "roq/htx/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace htx {
namespace protocol {
namespace json {

// === CONSTANTS ===

namespace {
constexpr auto const KEY_ACTION = "action"sv;
constexpr auto const KEY_CH = "ch"sv;
constexpr auto const KEY_MESSAGE = "message"sv;
constexpr auto const KEY_PING = "ping"sv;
constexpr auto const KEY_REP = "rep"sv;
constexpr auto const KEY_STATUS = "status"sv;
constexpr auto const KEY_SUBBED = "subbed"sv;
}  // namespace

// === HELPERS ===

namespace {
template <typename T>
auto dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
  return true;
}

constexpr auto extract_topic(std::string_view const &channel) {
  if (channel == "orders#*"sv) {
    return "orders"sv;
  }
  if (channel == "trade.clearing#*"sv) {
    return "clearing"sv;
  }
  auto sep1 = channel.find_first_of('.');
  if (sep1 != channel.npos) {
    auto tmp1 = channel.substr(0, sep1);
    if (tmp1 == "accounts"sv) {
      return tmp1;
    }
    ++sep1;
    auto sep2 = channel.find_first_of('.', sep1);
    if (sep2 != channel.npos) {
      ++sep2;
      auto sep3 = channel.find_first_of('.', sep2);
      if (sep3 != channel.npos) {
        return channel.substr(sep2, sep3 - sep2);
      }
      return channel.substr(sep2);
    }
    return channel.substr(sep1);
  }
  return channel;
}

static_assert(extract_topic("accounts.update"sv) == "accounts"sv);
static_assert(extract_topic("auth"sv) == "auth"sv);
static_assert(extract_topic("market.btcusdt.bbo"sv) == "bbo"sv);
static_assert(extract_topic("market.btcusdt.detail"sv) == "detail"sv);
static_assert(extract_topic("market.btcusdt.mbp.20"sv) == "mbp"sv);
static_assert(extract_topic("market.btcusdt.ticker"sv) == "ticker"sv);
static_assert(extract_topic("market.btcusdt.trade.detail"sv) == "trade"sv);
static_assert(extract_topic("orders#*"sv) == "orders"sv);
static_assert(extract_topic("trade.clearing#*"sv) == "clearing"sv);
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(
    Parser::Handler &handler,
    std::string_view const &message,
    core::json::BufferStack &buffer_stack,
    TraceInfo const &trace_info,
    bool allow_unknown_event_types) {
  auto result = false;
  auto helper = [&](auto &key, auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_ACTION): {
        Action action{value};
        switch (action) {
          using enum Action::type_t;
          case UNDEFINED_INTERNAL:
            log::fatal("Unexpected"sv);
          case UNKNOWN_INTERNAL:
            return true;
          case SUB:
            result = dispatch_helper<protocol::json::Sub>(handler, message, buffer_stack, trace_info);
            break;
          case REQ:
            result = dispatch_helper<protocol::json::Req>(handler, message, buffer_stack, trace_info);
            break;
          case PING:
            result = dispatch_helper<protocol::json::Ping>(handler, message, buffer_stack, trace_info);
            break;
          case PONG:
            // result = dispatch_helper<protocol::json::Pong>(handler, message, buffer_stack, trace_info);
            return true;
          case PUSH:
            break;  // need channel
        }
        break;
      }
      case utils::hash::FNV::compute(KEY_CH): {
        Topic topic{extract_topic(std::get<std::string_view>(value))};
        switch (topic) {
          using enum Topic::type_t;
          case UNDEFINED_INTERNAL:
            log::fatal("Unexpected"sv);
          case UNKNOWN_INTERNAL:
            return true;
          case BBO:
            result = dispatch_helper<protocol::json::BBO>(handler, message, buffer_stack, trace_info);
            break;
          case TRADE:
            result = dispatch_helper<Trade>(handler, message, buffer_stack, trace_info);
            break;
          case DETAIL:
            result = dispatch_helper<Detail>(handler, message, buffer_stack, trace_info);
            break;
          case TICKER:
            result = dispatch_helper<Ticker>(handler, message, buffer_stack, trace_info);
            break;
          case MBP:
            result = dispatch_helper<protocol::json::MBP>(handler, message, buffer_stack, trace_info);
            break;
          case AUTH:
            // result = dispatch_helper<protocol::json::Auth>(handler, message, buffer_stack, trace_info);
            return true;
            break;
          case ACCOUNTS:
            result = dispatch_helper<Accounts>(handler, message, buffer_stack, trace_info);
            return true;
          case ORDERS:
            result = dispatch_helper<Orders>(handler, message, buffer_stack, trace_info);
            return true;
          case CLEARING:
            result = dispatch_helper<Clearing>(handler, message, buffer_stack, trace_info);
            return true;
        }
        break;
      }
      case utils::hash::FNV::compute(KEY_MESSAGE): {
        result = dispatch_helper<Error2>(handler, message, buffer_stack, trace_info);
        break;
      }
      case utils::hash::FNV::compute(KEY_PING): {
        result = dispatch_helper<Ping2>(handler, message, buffer_stack, trace_info);
        break;
      }
      case utils::hash::FNV::compute(KEY_REP): {
        Topic topic{extract_topic(std::get<std::string_view>(value))};
        if (topic == Topic::MBP) {
          result = dispatch_helper<MBPSnapshot>(handler, message, buffer_stack, trace_info);
        }
        break;
      }
      case utils::hash::FNV::compute(KEY_STATUS): {
        Status status{value};
        switch (status) {
          using enum Status::type_t;
          case UNDEFINED_INTERNAL:
            log::fatal("Unexpected"sv);
          case UNKNOWN_INTERNAL:
            return true;
          case OK:
            break;  // note! continue
          case ERROR:
            result = dispatch_helper<protocol::json::Error>(handler, message, buffer_stack, trace_info);
            break;
        }
        break;
      }
      case utils::hash::FNV::compute(KEY_SUBBED): {
        result = dispatch_helper<Subbed>(handler, message, buffer_stack, trace_info);
        break;
      }
    }
    return result;
  };
  core::json::Parser::dispatch<core::json::Object>(helper, message);
  if (result || allow_unknown_event_types) {
    return result;
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace protocol
}  // namespace htx
}  // namespace roq
