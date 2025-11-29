/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/sub.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("success", "[json_sub]") {
  auto message = R"({)"
                 R"("action":"sub",)"
                 R"("code":200,)"
                 R"("ch":"trade.clearing#*",)"
                 R"("data":{})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Sub obj{message, buffer};
}
