/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/htx/json/parser.hpp"

#include <algorithm>
#include <cctype>
#include <string>

#include "roq/compat.hpp"

#include "roq/logging.hpp"

#include "roq/utils/hash/fnv.hpp"

#include "roq/htx/json/topic.hpp"
#include "roq/htx/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace htx {
namespace json {

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(
    Parser::Handler &handler,
    std::string_view const &message,
    core::json::BufferStack &buffer_stack,
    TraceInfo const &trace_info,
    bool allow_unknown_event_types) {
  auto stop = false;
  auto result = false;
  auto helper = [&](auto &key, auto &value) {
    if (result || stop) {  // XXX FIXME TODO can we stop early?
      return;
    }
    auto k = utils::hash::FNV::compute(key);
    switch (k) {
      case utils::hash::FNV::compute("action"sv): {
        Action action{std::get<std::string_view>(value)};
        switch (action) {
          using enum Action::type_t;
          case UNDEFINED_INTERNAL:
            break;  // XXX stop?
          case UNKNOWN_INTERNAL:
            if (allow_unknown_event_types) {
              stop = true;
            }
            break;
          case SUB:
            result = true;
            dispatch_helper<json::Sub>(handler, message, buffer_stack, trace_info);
            break;
          case REQ:
            result = true;
            dispatch_helper<json::Req>(handler, message, buffer_stack, trace_info);
            break;
          case PING:
            result = true;
            dispatch_helper<json::Ping>(handler, message, buffer_stack, trace_info);
            break;
          case PONG:
            stop = true;
            // dispatch_helper<json::Pong>(handler, message, buffer_stack, trace_info);
            break;
          case PUSH:
            stop = true;
            // dispatch_helper<json::Push>(handler, message, buffer_stack, trace_info);
            break;
        }
        break;
      }
      case utils::hash::FNV::compute("ch"sv): {
        Topic topic{extract_topic(std::get<std::string_view>(value))};
        switch (topic) {
          using enum Topic::type_t;
          case UNDEFINED_INTERNAL:
            break;
          case UNKNOWN_INTERNAL:
            if (allow_unknown_event_types) {
              stop = true;
            }
            break;
          case BBO:
            result = true;
            dispatch_helper<json::BBO>(handler, message, buffer_stack, trace_info);
            break;
          case TRADE:
            result = true;
            dispatch_helper<Trade>(handler, message, buffer_stack, trace_info);
            break;
          case DETAIL:
            result = true;
            dispatch_helper<Detail>(handler, message, buffer_stack, trace_info);
            break;
          case TICKER:
            result = true;
            dispatch_helper<Ticker>(handler, message, buffer_stack, trace_info);
            break;
          case MBP:
            result = true;
            dispatch_helper<json::MBP>(handler, message, buffer_stack, trace_info);
            break;
          case AUTH:
            // result = true;
            // dispatch_helper<json::Auth>(handler, message, buffer_stack, trace_info);
            break;
        }
        break;
      }
      case utils::hash::FNV::compute("message"sv): {
        result = true;
        dispatch_helper<Error2>(handler, message, buffer_stack, trace_info);
        break;
      }
      case utils::hash::FNV::compute("ping"sv): {
        result = true;
        dispatch_helper<Ping2>(handler, message, buffer_stack, trace_info);
        break;
      }
      case utils::hash::FNV::compute("rep"sv): {
        Topic topic{extract_topic(std::get<std::string_view>(value))};
        if (topic == Topic::MBP) {
          result = true;
          dispatch_helper<MBPSnapshot>(handler, message, buffer_stack, trace_info);
        }
        break;
      }
      case utils::hash::FNV::compute("status"sv): {
        Status status{std::get<std::string_view>(value)};
        switch (status) {
          using enum Status::type_t;
          case UNDEFINED_INTERNAL:
            break;  // XXX stop?
          case UNKNOWN_INTERNAL:
            if (allow_unknown_event_types) {
              stop = true;
            }
            break;
          case OK:
            break;  // note! continue
          case ERROR:
            result = true;
            dispatch_helper<json::Error>(handler, message, buffer_stack, trace_info);
        }
        break;
      }
      case utils::hash::FNV::compute("subbed"sv): {
        result = true;
        dispatch_helper<Subbed>(handler, message, buffer_stack, trace_info);
        break;
      }
    }
    /*
    if (!frame.ping.count()) {
      switch (frame.status) {
        using enum Status::type_t;
        case UNDEFINED_INTERNAL: {
          Topic topic{extract_topic(frame.ch)};
          switch (topic) {
            using enum Topic::type_t;
            case UNDEFINED_INTERNAL:
              break;
            case UNKNOWN_INTERNAL:
              if (allow_unknown_event_types) {
                return false;
              }
              break;
            case BBO:
              dispatch_helper<json::BBO>(handler, message, buffer_stack, trace_info);
              return true;
            case TRADE:
              dispatch_helper<Trade>(handler, message, buffer_stack, trace_info);
              return true;
            case DETAIL:
              dispatch_helper<Detail>(handler, message, buffer_stack, trace_info);
              return true;
            case TICKER:
              dispatch_helper<Ticker>(handler, message, buffer_stack, trace_info);
              return true;
            case MBP:
              dispatch_helper<json::MBP>(handler, message, buffer_stack, trace_info);
              return true;
          }
          break;
        }
        case UNKNOWN_INTERNAL:
          if (allow_unknown_event_types) {
            return false;
          }
          break;
        case OK:
          if (!std::empty(frame.subbed)) {
            auto subbed = Subbed{
                .id = frame.id,
                .subbed = frame.subbed,
                .ts = frame.ts,
                .status = frame.status,
            };
            create_trace_and_dispatch(handler, trace_info, subbed);
            return true;
          } else {
            if (!std::empty(frame.rep)) {
              Topic topic{extract_topic(frame.rep)};
              if (topic == Topic::MBP) {
                dispatch_helper<MBPSnapshot>(handler, message, buffer_stack, trace_info);
                return true;
              }
            }
            log::fatal("DEBUG {}"sv, message);  // ???
          }
          break;
        case ERROR:
          auto error = Error{
              .id = frame.id,
              .err_code = frame.err_code,
              .err_msg = frame.err_msg,
              .ts = frame.ts,
          };
          create_trace_and_dispatch(handler, trace_info, error);
          return true;
      }
    } else {
      auto ping = Ping{
          .timestamp = frame.ping,
      };
      create_trace_and_dispatch(handler, trace_info, ping);
      return true;
    }
    */
  };
  core::json::Parser parser{message};
  auto value = parser.root();
  std::get<core::json::Object>(value).dispatch(helper);
  if (result || stop) {
    return result;
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace htx
}  // namespace roq
