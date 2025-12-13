/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/accounts_ack.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::AccountsAck;

TEST_CASE("simple", "[json_accounts_ack]") {
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
  auto helper = [&](value_type &obj) { CHECK(obj.status == json::Status::OK); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
