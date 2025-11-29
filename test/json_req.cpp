/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/req.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("auth_success", "[json_req]") {
  auto message = R"({)"
                 R"("action":"req",)"
                 R"("code":200,)"
                 R"("ch":"auth",)"
                 R"("data":{})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Req obj{message, buffer};
}

TEST_CASE("auth_error", "[json_req]") {
  auto message = R"({)"
                 R"("action":"req",)"
                 R"("code":2001,)"
                 R"("ch":"auth",)"
                 R"("message":"invalid.authType")"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Req obj{message, buffer};
}
