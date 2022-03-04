/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/json/parser.h"

#include "roq/huobi/json/trade.h"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_trade_simple", "[json_trade]") {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.trade.detail",)"
                 R"("ts":1639670532436,)"
                 R"("tick":{)"
                 R"("id":144446879688,)"
                 R"("ts":1639670532434,)"
                 R"("data":[{)"
                 R"("id":144446879688433873170760015,)"
                 R"("ts":1639670532434,)"
                 R"("tradeId":102579820014,)"
                 R"("amount":4.74E-4,)"
                 R"("price":48542.5,)"
                 R"("direction":"sell")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Trade>(message, buffer_);
}
