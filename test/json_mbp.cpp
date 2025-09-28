/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/mbp.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_mbp_simple", "[json_mbp]") {
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
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::MBP mbp{message, buffer};
}
