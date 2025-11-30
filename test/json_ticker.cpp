/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/htx/json/parser.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_ticker]") {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.ticker",)"
                 R"("ts":1639670087122,)"
                 R"("tick":{)"
                 R"("open":46749.25,)"
                 R"("high":49500.0,)"
                 R"("low":46533.38,)"
                 R"("close":48535.19,)"
                 R"("amount":10435.58445581327,)"
                 R"("vol":5.0629586810098803E8,)"
                 R"("count":799781,)"
                 R"("bid":48535.18,)"
                 R"("bidSize":0.008885,)"
                 R"("ask":48535.19,)"
                 R"("askSize":0.28117180436462696,)"
                 R"("lastPrice":48535.19,)"
                 R"("lastSize":1.08E-4)"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  // simple
  json::Ticker obj{message, buffers};
  CHECK(obj.ch == "market.btcusdt.ticker"sv);
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
    void operator()(Trace<json::Detail> const &) override { FAIL(); }
    void operator()(Trace<json::Ticker> const &event) override {
      found = true;
      auto &[trace_info, ticker] = event;
      CHECK(ticker.ch == "market.btcusdt.ticker"sv);
    }
    void operator()(Trace<json::MBP> const &) override { FAIL(); }
    void operator()(Trace<json::MBPSnapshot> const &) override { FAIL(); }

    bool found = false;
  } handler;
  auto res = json::Parser::dispatch(handler, message, buffers, {}, false);
  CHECK(res == true);
  CHECK(handler.found == true);
}
