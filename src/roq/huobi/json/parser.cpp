/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/huobi/json/parser.hpp"

#include <algorithm>
#include <cctype>
#include <string>

#include "roq/compat.hpp"

#include "roq/logging.hpp"

#include "roq/huobi/json/frame.hpp"
#include "roq/huobi/json/topic.hpp"
#include "roq/huobi/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace huobi {
namespace json {

bool Parser::dispatch(
    Parser::Handler &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  auto frame = core::json::Parser::create<json::Frame>(message, buffer);
  if (!frame.ping.count()) {
    switch (frame.status) {
      using enum Status::type_t;
      case UNDEFINED: {
        Topic topic{extract_topic(frame.ch)};
        switch (topic) {
          using enum Topic::type_t;
          case BBO: {
            const auto bbo = core::json::Parser::create<json::BBO>(message, buffer);
            create_trace_and_dispatch(handler, trace_info, bbo);
            return true;
          }
          case TRADE: {
            const auto trade = core::json::Parser::create<json::Trade>(message, buffer);
            create_trace_and_dispatch(handler, trace_info, trade);
            return true;
          }
          case DETAIL: {
            const auto detail = core::json::Parser::create<json::Detail>(message, buffer);
            create_trace_and_dispatch(handler, trace_info, detail);
            return true;
          }
          case TICKER: {
            const auto ticker = core::json::Parser::create<json::Ticker>(message, buffer);
            create_trace_and_dispatch(handler, trace_info, ticker);
            return true;
          }
          case MBP: {
            const auto mbp = core::json::Parser::create<json::MBP>(message, buffer);
            create_trace_and_dispatch(handler, trace_info, mbp);
            return true;
          }
          default:
            break;
        }
        break;
      }
      case UNKNOWN:
        break;
      case OK:
        if (!std::empty(frame.subbed)) {
          const Subbed subbed{
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
              const auto mbp_snapshot =
                  core::json::Parser::create<json::MBPSnapshot>(message, buffer);
              create_trace_and_dispatch(handler, trace_info, mbp_snapshot);
              return true;
            }
          }
          log::fatal("DEBUG {}"sv, message);  // ???
        }
        break;
      case ERROR:
        const Error error{
            .id = frame.id,
            .err_code = frame.err_code,
            .err_msg = frame.err_msg,
            .ts = frame.ts,
        };
        create_trace_and_dispatch(handler, trace_info, error);
        return true;
    }
  } else {
    const Ping ping{
        .timestamp = frame.ping,
    };
    create_trace_and_dispatch(handler, trace_info, ping);
    return true;
  }
  log::warn(R"(Unexpected: message="{}")"sv, message);
  return false;
}

}  // namespace json
}  // namespace huobi
}  // namespace roq
