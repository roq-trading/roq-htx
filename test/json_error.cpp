/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/error.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_error]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("ts":1764300263709,)"
                 R"("id":"5000001",)"
                 R"("err-code":"bad-request",)"
                 R"("err-msg":"The coin pair does not currently offer subscription services.")"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Error obj{message, buffer};
}
