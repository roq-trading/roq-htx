/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/huobi/json/symbols.h"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

// note! heavily truncated
TEST(json_symbols, simple) {
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
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::Symbols>(message, buffer);
  EXPECT_EQ(obj.status, "ok"sv);
  auto &data = obj.data;
  ASSERT_EQ(std::size(data), 2);
  auto &d0 = data[0];
  EXPECT_EQ(d0.base_currency, "stk"sv);
  EXPECT_EQ(d0.quote_currency, "eth"sv);
  EXPECT_EQ(d0.price_precision, 8);
  EXPECT_EQ(d0.amount_precision, 2);
  EXPECT_EQ(d0.symbol_partition, json::Partition::INNOVATION);
  EXPECT_EQ(d0.symbol, "stketh"sv);
  EXPECT_EQ(d0.state, json::State::OFFLINE);
  EXPECT_EQ(d0.value_precision, 8);
  EXPECT_DOUBLE_EQ(d0.min_order_amt, 1.0);
  EXPECT_DOUBLE_EQ(d0.max_order_amt, 10000000.0);
  EXPECT_DOUBLE_EQ(d0.min_order_value, 0.01);
  EXPECT_DOUBLE_EQ(d0.limit_order_min_order_amt, 1.0);
  EXPECT_DOUBLE_EQ(d0.limit_order_max_order_amt, 10000000.0);
  EXPECT_DOUBLE_EQ(d0.limit_order_max_buy_amt, 10000000.0);
  EXPECT_DOUBLE_EQ(d0.limit_order_max_sell_amt, 10000000.0);
  EXPECT_DOUBLE_EQ(d0.sell_market_min_order_amt, 1.0);
  EXPECT_DOUBLE_EQ(d0.sell_market_max_order_amt, 1000000.0);
  EXPECT_DOUBLE_EQ(d0.buy_market_max_order_value, 500.0);
  EXPECT_EQ(d0.api_trading, json::Trading::ENABLED);
  EXPECT_EQ(d0.tags, ""sv);
  auto &d1 = data[1];
  EXPECT_EQ(d1.base_currency, "poly"sv);
  EXPECT_EQ(d1.quote_currency, "eth"sv);
  EXPECT_EQ(d1.price_precision, 6);
  EXPECT_EQ(d1.amount_precision, 4);
  EXPECT_EQ(d1.symbol_partition, json::Partition::INNOVATION);
  EXPECT_EQ(d1.symbol, "polyeth"sv);
  EXPECT_EQ(d1.state, json::State::ONLINE);
  EXPECT_EQ(d1.value_precision, 8);
  EXPECT_DOUBLE_EQ(d1.min_order_amt, 0.1);
  EXPECT_DOUBLE_EQ(d1.max_order_amt, 1000000.0);
  EXPECT_DOUBLE_EQ(d1.min_order_value, 0.01);
  EXPECT_DOUBLE_EQ(d1.limit_order_min_order_amt, 0.1);
  EXPECT_DOUBLE_EQ(d1.limit_order_max_order_amt, 1000000.0);
  EXPECT_DOUBLE_EQ(d1.limit_order_max_buy_amt, 1000000.0);
  EXPECT_DOUBLE_EQ(d1.limit_order_max_sell_amt, 1000000.0);
  EXPECT_DOUBLE_EQ(d1.sell_market_min_order_amt, 0.1);
  EXPECT_DOUBLE_EQ(d1.sell_market_max_order_amt, 100000.0);
  EXPECT_DOUBLE_EQ(d1.buy_market_max_order_value, 50.0);
  EXPECT_EQ(d1.api_trading, json::Trading::ENABLED);
  EXPECT_EQ(d1.tags, ""sv);
}
