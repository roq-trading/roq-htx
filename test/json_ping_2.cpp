/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/ping_2.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_ping]") {
  auto message = R"({)"
                 R"("ping":1764300452378)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Ping2 obj{message, buffer};
}
