/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/protocol/json/cancel_order_ack.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::CancelOrderAck;

TEST_CASE("success", "[json_cancel_order_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("data":"1474909653904303")"
                 R"(})";
  auto helper = [&](value_type &obj) { CHECK(obj.status == protocol::json::Status::OK); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
