/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/json/parser.h"

#include "roq/huobi/json/bbo.h"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_bbo_simple", "[json_bbo]") {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.bbo",)"
                 R"("ts":1639669832793,)"
                 R"("tick":{)"
                 R"("seqId":144446084385,)"
                 R"("ask":48660.95,)"
                 R"("askSize":0.001298,)"
                 R"("bid":48660.94,)"
                 R"("bidSize":0.228303,)"
                 R"("quoteTime":1639669832791,)"
                 R"("symbol":"btcusdt")"
                 R"(})"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::BBO>(message, buffer_);
}
