/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/place_order_ack.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::PlaceOrderAck;

TEST_CASE("success", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("data":"1474908579893191",)"
                 R"("clientOrderId":"bAACeMp9pEMAAQAAAAAA")"
                 R"(})";
  auto helper = [&](value_type &obj) { CHECK(obj.status == json::Status::OK); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("missing_account_id", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("err-code":"require-account-id",)"
                 R"("err-msg":"Parameter `account-id` is required.",)"
                 R"("data":null)"
                 R"(})";
  auto helper = [&](value_type &obj) { CHECK(obj.status == json::Status::ERROR); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("order_min_value", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("err-code":"order-value-min-error",)"
                 R"("err-msg":"Order total cannot be lower than: 5 USDT",)"
                 R"("data":null)"
                 R"(})";
  auto helper = [&](value_type &obj) { CHECK(obj.status == json::Status::ERROR); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("not_enough_balance", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("err-code":"account-frozen-balance-insufficient-error",)"
                 R"("err-msg":"trade account balance is not enough, left: 0",)"
                 R"("data":null)"
                 R"(})";
  auto helper = [&](value_type &obj) { CHECK(obj.status == json::Status::ERROR); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("inexistent", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("err-code":"account-frozen-account-inexistent-error",)"
                 R"("err-msg":"account for id 68,824,237 and user id 54,918,049 does not exist",)"
                 R"("data":null)"
                 R"(})";
  auto helper = [&](value_type &obj) { CHECK(obj.status == json::Status::ERROR); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
