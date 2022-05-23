/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/huobi/json/ticker.hpp"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_ticker_simple", "[json_ticker]") {
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
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Ticker>(message, buffer_);
}
