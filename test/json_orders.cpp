/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Orders;

TEST_CASE("submitted", "[json_orders]") {
  auto message = R"({)"
                 R"("action":"push",)"
                 R"("ch":"orders#*",)"
                 R"("data":{)"
                 R"("orderCreateTime":1764741859504,)"
                 R"("totalTradeAmount":"0",)"
                 R"("orderSource":"spot-api",)"
                 R"("accountId":68824237,)"
                 R"("orderPrice":"32000",)"
                 R"("orderSize":"0.001",)"
                 R"("eventType":"creation",)"
                 R"("symbol":"btcusdt",)"
                 R"("orderStatus":"submitted",)"
                 R"("orderId":1474908579893191,)"
                 R"("clientOrderId":"bAACeMp9pEMAAQAAAAAA",)"
                 R"("type":"buy-limit")"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::PUSH);
    CHECK(obj.ch == "orders#*"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("canceled", "[json_orders]") {
  auto message = R"({)"
                 R"("action":"push",)"
                 R"("ch":"orders#*",)"
                 R"("data":{)"
                 R"("execAmt":"0",)"
                 R"("lastActTime":1764743913799,)"
                 R"("orderPrice":"32000",)"
                 R"("orderSize":"0.001",)"
                 R"("remainAmt":"0.001",)"
                 R"("orderSource":"spot-api",)"
                 R"("totalTradeAmount":"0",)"
                 R"("canceledSource":"",)"
                 R"("canceledSourceDesc":"",)"
                 R"("symbol":"btcusdt",)"
                 R"("eventType":"cancellation",)"
                 R"("clientOrderId":"bAACeMp9pEMAAQAAAAAA",)"
                 R"("orderStatus":"canceled",)"
                 R"("orderId":1474908579893191,)"
                 R"("type":"buy-limit")"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::PUSH);
    CHECK(obj.ch == "orders#*"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("filled", "[json_orders]") {
  auto message = R"({)"
                 R"("action":"push",)"
                 R"("ch":"orders#*",)"
                 R"("data":{)"
                 R"("tradePrice":"92800",)"
                 R"("aggressor":false,)"
                 R"("tradeVolume":"0.0001",)"
                 R"("tradeTime":1764753390751,)"
                 R"("execAmt":"0.0001",)"
                 R"("tradeId":103617355903,)"
                 R"("orderSource":"spot-api",)"
                 R"("orderPrice":"92800",)"
                 R"("orderSize":"0.0001",)"
                 R"("remainAmt":"0",)"
                 R"("totalTradeAmount":"9.2800",)"
                 R"("symbol":"btcusdt",)"
                 R"("eventType":"trade",)"
                 R"("clientOrderId":"6QACxZoaq0MAAQAAAAAA",)"
                 R"("orderStatus":"filled",)"
                 R"("orderId":1474910140509947,)"
                 R"("type":"buy-limit")"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::PUSH);
    CHECK(obj.ch == "orders#*"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
