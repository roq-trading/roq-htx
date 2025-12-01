/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/place_order_ack.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("missing_account_id", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("err-code":"require-account-id",)"
                 R"("err-msg":"Parameter `account-id` is required.",)"
                 R"("data":null)"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  json::PlaceOrderAck obj{message, buffers};
  CHECK(obj.status == json::Status::ERROR);
}

TEST_CASE("order_min_value", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("err-code":"order-value-min-error",)"
                 R"("err-msg":"Order total cannot be lower than: 5 USDT",)"
                 R"("data":null)"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  json::PlaceOrderAck obj{message, buffers};
  CHECK(obj.status == json::Status::ERROR);
}

TEST_CASE("not_enough_balance", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("err-code":"account-frozen-balance-insufficient-error",)"
                 R"("err-msg":"trade account balance is not enough, left: 0",)"
                 R"("data":null)"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  json::PlaceOrderAck obj{message, buffers};
  CHECK(obj.status == json::Status::ERROR);
}
