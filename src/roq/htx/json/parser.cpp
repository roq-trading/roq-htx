/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/htx/json/parser.hpp"

#include <algorithm>
#include <cctype>
#include <string>

#include "roq/compat.hpp"

#include "roq/logging.hpp"

#include "roq/htx/json/frame.hpp"
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
  Frame frame{message, buffer_stack};
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
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace htx
}  // namespace roq
