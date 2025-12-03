/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Clearing;

TEST_CASE("maker", "[json_clearing]") {
  auto message = R"({)"
                 R"("action":"push",)"
                 R"("ch":"trade.clearing#*",)"
                 R"("data":{)"
                 R"("eventType":"trade",)"
                 R"("symbol":"btcusdt",)"
                 R"("orderId":1474910140509947,)"
                 R"("orderSide":"buy",)"
                 R"("orderType":"buy-limit",)"
                 R"("accountId":68824237,)"
                 R"("source":"spot-api",)"
                 R"("orderPrice":"92800",)"
                 R"("orderSize":"0.0001",)"
                 R"("clientOrderId":"6QACxZoaq0MAAQAAAAAA",)"
                 R"("orderCreateTime":1764753031817,)"
                 R"("orderStatus":"filled",)"
                 R"("feeCurrency":"btc",)"
                 R"("tradePrice":"92800",)"
                 R"("tradeVolume":"0.0001",)"
                 R"("aggressor":false,)"
                 R"("tradeId":103617355903,)"
                 R"("tradeTime":1764753390751,)"
                 R"("transactFee":"0.0000002",)"
                 R"("feeDeduct":"0",)"
                 R"("feeDeductType":"")"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::PUSH);
    CHECK(obj.ch == "trade.clearing#*"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("taker", "[json_clearing]") {
  auto message = R"({)"
                 R"("action":"push",)"
                 R"("ch":"trade.clearing#*",)"
                 R"("data":{)"
                 R"("eventType":"trade",)"
                 R"("symbol":"btcusdt",)"
                 R"("orderId":1474910383802238,)"
                 R"("orderSide":"buy",)"
                 R"("orderType":"buy-limit",)"
                 R"("accountId":68824237,)"
                 R"("source":"spot-api",)"
                 R"("orderPrice":"93000",)"
                 R"("orderSize":"0.0001",)"
                 R"("clientOrderId":"GAAC3nAdrEMAAQAAAAAA",)"
                 R"("orderCreateTime":1764754899204,)"
                 R"("orderStatus":"filled",)"
                 R"("feeCurrency":"btc",)"
                 R"("tradePrice":"92951.16",)"
                 R"("tradeVolume":"0.0001",)"
                 R"("aggressor":true,)"
                 R"("tradeId":103617356763,)"
                 R"("tradeTime":1764754899208,)"
                 R"("transactFee":"0.0000002",)"
                 R"("feeDeduct":"0",)"
                 R"("feeDeductType":"")"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::PUSH);
    CHECK(obj.ch == "trade.clearing#*"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
