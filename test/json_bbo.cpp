/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/bbo.hpp"

using namespace roq;
using namespace roq::htx;

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
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::BBO bbo{message, buffer};
}
