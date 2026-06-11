/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/protocol/json/open_orders_ack.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::OpenOrdersAck;

TEST_CASE("empty", "[json_open_orders_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("data":[])"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.status == protocol::json::Status::OK);
    CHECK(std::empty(obj.data));
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("simple", "[json_open_orders_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("data":[{)"
                 R"("source":"api",)"
                 R"("symbol":"btcusdt",)"
                 R"("created-at":1764741859504,)"
                 R"("ice-amount":"0.0",)"
                 R"("client-order-id":"bAACeMp9pEMAAQAAAAAA",)"
                 R"("account-type":"spot",)"
                 R"("account-id":68824237,)"
                 R"("price":"32000.000000000000000000",)"
                 R"("amount":"0.001000000000000000",)"
                 R"("filled-cash-amount":"0.0",)"
                 R"("filled-fees":"0.0",)"
                 R"("filled-amount":"0.0",)"
                 R"("id":1474908579893191,)"
                 R"("state":"submitted",)"
                 R"("type":"buy-limit")"
                 R"(})"
                 R"(])"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.status == protocol::json::Status::OK);
    REQUIRE(std::size(obj.data) == 1);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
