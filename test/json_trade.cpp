/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Trade;

TEST_CASE("simple", "[json_trade]") {
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.ch == "market.btcusdt.trade.detail"sv);
    CHECK(obj.ts == 1639670532436ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
