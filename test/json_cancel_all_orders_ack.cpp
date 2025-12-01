/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/cancel_all_orders_ack.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("success", "[json_cancel_all_orders_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("data":true)"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  json::CancelAllOrdersAck obj{message, buffers};
  CHECK(obj.status == json::Status::OK);
}

TEST_CASE("failure", "[json_cancel_all_orders_ack]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("err-code":"login-required",)"
                 R"("err-msg":"The account is not logged in, please log in and try again.",)"
                 R"("data":null)"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  json::CancelAllOrdersAck obj{message, buffers};
  CHECK(obj.status == json::Status::ERROR);
}
