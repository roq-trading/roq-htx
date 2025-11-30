/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/htx/json/parser.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_error]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("ts":1764300263709,)"
                 R"("id":"5000001",)"
                 R"("err-code":"bad-request",)"
                 R"("err-msg":"The coin pair does not currently offer subscription services.")"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  // simple
  json::Error obj{message, buffers};
  CHECK(obj.status == json::Status::ERROR);
  // parser
  struct Handler final : public json::Parser::Handler {
    void operator()(Trace<json::Req> const &) override { FAIL(); }
    void operator()(Trace<json::Ping> const &) override { FAIL(); }
    void operator()(Trace<json::Ping2> const &) override { FAIL(); }
    void operator()(Trace<json::Error> const &event) override {
      found = true;
      auto &[trace_info, error] = event;
      CHECK(error.status == json::Status::ERROR);
    }
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
