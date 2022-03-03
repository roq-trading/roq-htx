/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/json/parser.h"

#include "roq/huobi/json/mbp.h"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_mbp_simple", "json_mbp") {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.mbp.20",)"
                 R"("ts":1639744309814,)"
                 R"("tick":{)"
                 R"("seqNum":144516551156,)"
                 R"("prevSeqNum":144516551150,)"
                 R"("bids":[)"
                 R"([47231.83,0.957689])"
                 R"(])"
                 R"(})"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::MBP>(message, buffer_);
}
