/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/ping.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_ping]") {
  auto message = R"({)"
                 R"("action":"ping",)"
                 R"("data":{)"
                 R"("ts":1764299053602)"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Ping obj{message, buffer};
}
