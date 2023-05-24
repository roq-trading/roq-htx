/* Copyright (c) 2017-2023, Hans Erik Thrane */

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
    std::string_view const &message,
    std::span<std::byte> const &buffer,
    TraceInfo const &trace_info) {
  auto frame = Frame::create(message, buffer);
  if (!frame.ping.count()) {
    switch (frame.status) {
      using enum Status::type_t;
      case UNDEFINED__: {
        Topic topic{extract_topic(frame.ch)};
        switch (topic) {
          using enum Topic::type_t;
          case BBO: {
            auto bbo = json::BBO::create(message, buffer);
            create_trace_and_dispatch(handler, trace_info, bbo);
            return true;
          }
          case TRADE: {
            auto trade = Trade::create(message, buffer);
            create_trace_and_dispatch(handler, trace_info, trade);
            return true;
          }
          case DETAIL: {
            auto detail = Detail::create(message, buffer);
            create_trace_and_dispatch(handler, trace_info, detail);
            return true;
          }
          case TICKER: {
            auto ticker = Ticker::create(message, buffer);
            create_trace_and_dispatch(handler, trace_info, ticker);
            return true;
          }
          case MBP: {
            auto mbp = MBP::create(message, buffer);
            create_trace_and_dispatch(handler, trace_info, mbp);
            return true;
          }
          default:
            break;
        }
        break;
      }
      case UNKNOWN__:
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
              auto mbp_snapshot = MBPSnapshot::create(message, buffer);
              create_trace_and_dispatch(handler, trace_info, mbp_snapshot);
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
  log::warn(R"(Unexpected: message="{}")"sv, message);
  return false;
}

}  // namespace json
}  // namespace huobi
}  // namespace roq
