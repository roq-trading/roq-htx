/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/symbols.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

// note! heavily truncated
TEST_CASE("simple", "[json_symbols]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("data":[{)"
                 R"("base-currency":"stk",)"
                 R"("quote-currency":"eth",)"
                 R"("price-precision":8,)"
                 R"("amount-precision":2,)"
                 R"("symbol-partition":"innovation",)"
                 R"("symbol":"stketh",)"
                 R"("state":"offline",)"
                 R"("value-precision":8,)"
                 R"("min-order-amt":1,)"
                 R"("max-order-amt":10000000,)"
                 R"("min-order-value":0.01,)"
                 R"("limit-order-min-order-amt":1,)"
                 R"("limit-order-max-order-amt":10000000,)"
                 R"("limit-order-max-buy-amt":10000000,)"
                 R"("limit-order-max-sell-amt":10000000,)"
                 R"("sell-market-min-order-amt":1,)"
                 R"("sell-market-max-order-amt":1000000,)"
                 R"("buy-market-max-order-value":500,)"
                 R"("api-trading":"enabled",)"
                 R"("tags":"")"
                 R"(},{)"
                 R"("base-currency":"poly",)"
                 R"("quote-currency":"eth",)"
                 R"("price-precision":6,)"
                 R"("amount-precision":4,)"
                 R"("symbol-partition":"innovation",)"
                 R"("symbol":"polyeth",)"
                 R"("state":"online",)"
                 R"("value-precision":8,)"
                 R"("min-order-amt":0.1,)"
                 R"("max-order-amt":1000000,)"
                 R"("min-order-value":0.01,)"
                 R"("limit-order-min-order-amt":0.1,)"
                 R"("limit-order-max-order-amt":1000000,)"
                 R"("limit-order-max-buy-amt":1000000,)"
                 R"("limit-order-max-sell-amt":1000000,)"
                 R"("sell-market-min-order-amt":0.1,)"
                 R"("sell-market-max-order-amt":100000,)"
                 R"("buy-market-max-order-value":50,)"
                 R"("api-trading":"enabled",)"
                 R"("tags":"")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  json::Symbols obj{message, buffers};
  CHECK(obj.status == json::Status::OK);
  auto &data = obj.data;
  REQUIRE(std::size(data) == 2);
  auto &d0 = data[0];
  CHECK(d0.base_currency == "stk"sv);
  CHECK(d0.quote_currency == "eth"sv);
  CHECK(d0.price_precision == 8);
  CHECK(d0.amount_precision == 2);
  CHECK(d0.symbol_partition == json::Partition::INNOVATION);
  CHECK(d0.symbol == "stketh"sv);
  CHECK(d0.state == json::State::OFFLINE);
  CHECK(d0.value_precision == 8);
  CHECK(d0.min_order_amt == 1.0_a);
  CHECK(d0.max_order_amt == 10000000.0_a);
  CHECK(d0.min_order_value == 0.01_a);
  CHECK(d0.limit_order_min_order_amt == 1.0_a);
  CHECK(d0.limit_order_max_order_amt == 10000000.0_a);
  CHECK(d0.limit_order_max_buy_amt == 10000000.0_a);
  CHECK(d0.limit_order_max_sell_amt == 10000000.0_a);
  CHECK(d0.sell_market_min_order_amt == 1.0_a);
  CHECK(d0.sell_market_max_order_amt == 1000000.0_a);
  CHECK(d0.buy_market_max_order_value == 500.0_a);
  CHECK(d0.api_trading == json::Trading::ENABLED);
  CHECK(d0.tags == ""sv);
  auto &d1 = data[1];
  CHECK(d1.base_currency == "poly"sv);
  CHECK(d1.quote_currency == "eth"sv);
  CHECK(d1.price_precision == 6);
  CHECK(d1.amount_precision == 4);
  CHECK(d1.symbol_partition == json::Partition::INNOVATION);
  CHECK(d1.symbol == "polyeth"sv);
  CHECK(d1.state == json::State::ONLINE);
  CHECK(d1.value_precision == 8);
  CHECK(d1.min_order_amt == 0.1_a);
  CHECK(d1.max_order_amt == 1000000.0_a);
  CHECK(d1.min_order_value == 0.01_a);
  CHECK(d1.limit_order_min_order_amt == 0.1_a);
  CHECK(d1.limit_order_max_order_amt == 1000000.0_a);
  CHECK(d1.limit_order_max_buy_amt == 1000000.0_a);
  CHECK(d1.limit_order_max_sell_amt == 1000000.0_a);
  CHECK(d1.sell_market_min_order_amt == 0.1_a);
  CHECK(d1.sell_market_max_order_amt == 100000.0_a);
  CHECK(d1.buy_market_max_order_value == 50.0_a);
  CHECK(d1.api_trading == json::Trading::ENABLED);
  CHECK(d1.tags == ""sv);
}
