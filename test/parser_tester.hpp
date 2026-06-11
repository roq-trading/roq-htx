/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/htx/protocol/json/parser.hpp"

namespace roq {
namespace htx {

template <typename T>
struct ParserTester final : public protocol::json::Parser::Handler {
  using value_type = std::remove_cvref_t<T>;
  using callback_type = std::function<void(value_type const &)>;

  static void dispatch(callback_type const &callback, std::string_view const &message, size_t buffer_size, size_t max_depth) {
    core::json::BufferStack buffers{buffer_size, max_depth};
    // simple
    // XXX FIXME TODO catch2 block ???
    T obj{message, buffers};
    callback(obj);
    // parser
    // XXX FIXME TODO catch2 block ???
    ParserTester handler{callback};
    auto res = protocol::json::Parser::dispatch(handler, message, buffers, {}, false);
    CHECK(res == true);
    CHECK(handler.found_ == true);
  }

 protected:
  explicit ParserTester(callback_type const &callback) : callback_{callback} {}

  void operator()(Trace<protocol::json::Req> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Ping> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Ping2> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Error> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Error2> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Sub> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Subbed> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::BBO> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Trade> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Detail> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Ticker> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::MBP> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::MBPSnapshot> const &event) override { dispatch(event); }
  //
  void operator()(Trace<protocol::json::Accounts> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Orders> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Clearing> const &event) override { dispatch(event); }

  template <typename U>
  void dispatch(Trace<U> const &event) {
    if constexpr (std::is_invocable_v<callback_type, U>) {
      found_ = true;
      callback_(event);
    } else {
      FAIL();
    }
  }

 private:
  callback_type const callback_;
  bool found_ = false;
};

}  // namespace htx
}  // namespace roq
