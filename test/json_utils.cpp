/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/huobi/json/utils.h"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_utils, extract_symbol) {
  EXPECT_EQ(json::extract_symbol("market.btcusdt.ticker"sv), "btcusdt"sv);
  EXPECT_EQ(json::extract_symbol("market.btcusdt.bbo"sv), "btcusdt"sv);
  EXPECT_EQ(json::extract_symbol("market.btcusdt.trade.detail"sv), "btcusdt"sv);
  EXPECT_EQ(json::extract_symbol("market.btcusdt.detail"sv), "btcusdt"sv);
}

TEST(json_utils, extract_topic) {
  EXPECT_EQ(json::extract_topic("market.btcusdt.ticker"sv), "ticker"sv);
  EXPECT_EQ(json::extract_topic("market.btcusdt.bbo"sv), "bbo"sv);
  EXPECT_EQ(json::extract_topic("market.btcusdt.trade.detail"sv), "trade"sv);
  EXPECT_EQ(json::extract_topic("market.btcusdt.detail"sv), "detail"sv);
}
