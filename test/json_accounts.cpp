/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/accounts.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_accounts]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("data":[{)"
                 R"("id":71463752,)"
                 R"("type":"spot",)"
                 R"("subtype":"",)"
                 R"("state":"working")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Accounts obj{message, buffer};
}
