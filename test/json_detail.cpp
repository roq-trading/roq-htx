/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/huobi/json/detail.h"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_detail, simple) {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.detail",)"
                 R"("ts":1639672356380,)"
                 R"("tick":{)"
                 R"("id":288812213349,)"
                 R"("low":46641.43,)"
                 R"("high":49500.0,)"
                 R"("open":46879.92,)"
                 R"("close":48484.87,)"
                 R"("vol":5.009502243290115E8,)"
                 R"("amount":10312.405491742371,)"
                 R"("version":288812213349,)"
                 R"("count":795572)"
                 R"(})"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Detail>(message, buffer_);
}
