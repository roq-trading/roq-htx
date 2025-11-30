/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/htx/json/parser.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("auth_success", "[json_req]") {
  auto message = R"({)"
                 R"("action":"req",)"
                 R"("code":200,)"
                 R"("ch":"auth",)"
                 R"("data":{})"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  // simple
  json::Req obj{message, buffers};
  CHECK(obj.action == json::Action::REQ);
  // parser
  struct Handler final : public json::Parser::Handler {
    void operator()(Trace<json::Req> const &event) override {
      found = true;
      auto &[trace_info, req] = event;
      CHECK(req.action == json::Action::REQ);
    }
    void operator()(Trace<json::Ping> const &) override { FAIL(); }
    void operator()(Trace<json::Ping2> const &) override { FAIL(); }
    void operator()(Trace<json::Error> const &) override { FAIL(); }
    void operator()(Trace<json::Error2> const &) override { FAIL(); }
    void operator()(Trace<json::Sub> const &) override { FAIL(); }
    void operator()(Trace<json::Subbed> const &) override { FAIL(); }
    void operator()(Trace<json::BBO> const &) override { FAIL(); }
    void operator()(Trace<json::Trade> const &) override { FAIL(); }
    void operator()(Trace<json::Detail> const &) override { FAIL(); }
    void operator()(Trace<json::Ticker> const &) override { FAIL(); }
    void operator()(Trace<json::MBP> const &) override { FAIL(); }
    void operator()(Trace<json::MBPSnapshot> const &) override { FAIL(); }

    bool found = false;
  } handler;
  auto res = json::Parser::dispatch(handler, message, buffers, {}, false);
  CHECK(res == true);
  CHECK(handler.found == true);
}

TEST_CASE("auth_error", "[json_req]") {
  auto message = R"({)"
                 R"("action":"req",)"
                 R"("code":2001,)"
                 R"("ch":"auth",)"
                 R"("message":"invalid.authType")"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  // simple
  json::Req obj{message, buffers};
  CHECK(obj.action == json::Action::REQ);
  // parser
  struct Handler final : public json::Parser::Handler {
    void operator()(Trace<json::Req> const &event) override {
      found = true;
      auto &[trace_info, req] = event;
      CHECK(req.action == json::Action::REQ);
    }
    void operator()(Trace<json::Ping> const &) override { FAIL(); }
    void operator()(Trace<json::Ping2> const &) override { FAIL(); }
    void operator()(Trace<json::Error> const &) override { FAIL(); }
    void operator()(Trace<json::Error2> const &) override { FAIL(); }
    void operator()(Trace<json::Sub> const &) override { FAIL(); }
    void operator()(Trace<json::Subbed> const &) override { FAIL(); }
    void operator()(Trace<json::BBO> const &) override { FAIL(); }
    void operator()(Trace<json::Trade> const &) override { FAIL(); }
    void operator()(Trace<json::Detail> const &) override { FAIL(); }
    void operator()(Trace<json::Ticker> const &) override { FAIL(); }
    void operator()(Trace<json::MBP> const &) override { FAIL(); }
    void operator()(Trace<json::MBPSnapshot> const &) override { FAIL(); }

    bool found = false;
  } handler;
  auto res = json::Parser::dispatch(handler, message, buffers, {}, false);
  CHECK(res == true);
  CHECK(handler.found == true);
}
