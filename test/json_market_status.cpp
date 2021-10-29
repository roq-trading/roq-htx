/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/huobi/json/market_status.h"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_market_status, simple) {
  auto message = R"({)"
                 R"("code":200,)"
                 R"("message":"success",)"
                 R"("data":{)"
                 R"("marketStatus":1)"
                 R"(})"
                 R"(})";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::MarketStatus>(message, buffer);
  EXPECT_EQ(obj.message, "success"sv);
  auto &data = obj.data;
  EXPECT_EQ(data.market_status, 1);
}
