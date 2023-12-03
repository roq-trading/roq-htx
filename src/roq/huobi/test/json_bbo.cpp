/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/huobi/json/bbo.hpp"

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
  std::vector<std::byte> buffer(8192);
  [[maybe_unused]] auto obj = json::BBO::create(message, buffer);
}
