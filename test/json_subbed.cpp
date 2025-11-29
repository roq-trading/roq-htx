/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/subbed.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_subbed]") {
  auto message = R"({)"
                 R"("id":"5000001",)"
                 R"("status":"ok",)"
                 R"("subbed":"market.btcusdt.mbp.20",)"
                 R"("ts":1764298751689)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Subbed obj{message, buffer};
}
