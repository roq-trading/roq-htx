/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/huobi/json/mbp_snapshot.h"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

// note! reduced
TEST(json_mbp_snapshot, simple) {
  auto message = R"({)"
                 R"("id":"3000002",)"
                 R"("status":"ok",)"
                 R"("ts":1639751391156,)"
                 R"("rep":"market.btcusdt.mbp.20",)"
                 R"("data":{)"
                 R"("seqNum":144524033404,)"
                 R"("bids":[)"
                 R"([46705.1,0.211602],)"
                 R"([46703.6,0.038915])"
                 R"(],)"
                 R"("asks":[)"
                 R"([46705.11,0.196215],)"
                 R"([46709.03,0.008795])"
                 R"(])"
                 R"(})"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::MBPSnapshot>(message, buffer_);
}
