/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::Ticker;

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
  auto helper = [](value_type const &obj) {
    CHECK(obj.ch == "market.btcusdt.ticker"sv);
    CHECK(obj.ts == 1639670087122ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
