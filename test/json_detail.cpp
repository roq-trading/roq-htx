/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/htx/json/parser.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_detail]") {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.detail",)"
                 R"("ts":1639672356380,)"
                 R"("tick":{)"
                 R"("id":288812213349,)"
                 R"("low":46641.43,)"
                 R"("high":49500.0,)"
                 R"("open":46879.92,)"
                 R"("close":48484.87,)"
                 R"("vol":5.009502243290115E8,)"
                 R"("amount":10312.405491742371,)"
                 R"("version":288812213349,)"
                 R"("count":795572)"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  // simple
  json::Detail obj{message, buffers};
  CHECK(obj.ch == "market.btcusdt.detail"sv);
  // parser
  struct Handler final : public json::Parser::Handler {
    void operator()(Trace<json::Req> const &) override { FAIL(); }
    void operator()(Trace<json::Ping> const &) override { FAIL(); }
    void operator()(Trace<json::Ping2> const &) override { FAIL(); }
    void operator()(Trace<json::Error> const &) override { FAIL(); }
    void operator()(Trace<json::Error2> const &) override { FAIL(); }
    void operator()(Trace<json::Sub> const &) override { FAIL(); }
    void operator()(Trace<json::Subbed> const &) override { FAIL(); }
    void operator()(Trace<json::BBO> const &) override { FAIL(); }
    void operator()(Trace<json::Trade> const &) override { FAIL(); }
    void operator()(Trace<json::Detail> const &event) override {
      found = true;
      auto &[trace_info, detail] = event;
      CHECK(detail.ch == "market.btcusdt.detail"sv);
    }
    void operator()(Trace<json::Ticker> const &) override { FAIL(); }
    void operator()(Trace<json::MBP> const &) override { FAIL(); }
    void operator()(Trace<json::MBPSnapshot> const &) override { FAIL(); }

    bool found = false;
  } handler;
  auto res = json::Parser::dispatch(handler, message, buffers, {}, false);
  CHECK(res == true);
  CHECK(handler.found == true);
}
