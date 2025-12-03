/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/cancel_all_orders_ack.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::CancelAllOrdersAck;

TEST_CASE("success", "[json_cancel_all_orders_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("data":true)"
                 R"(})";
  auto helper = [&](value_type &obj) { CHECK(obj.status == json::Status::OK); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("failure", "[json_cancel_all_orders_ack]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("err-code":"login-required",)"
                 R"("err-msg":"The account is not logged in, please log in and try again.",)"
                 R"("data":null)"
                 R"(})";
  auto helper = [&](value_type &obj) { CHECK(obj.status == json::Status::ERROR); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
