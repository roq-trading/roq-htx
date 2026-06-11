/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/protocol/json/balance_ack.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::BalanceAck;

// note! truncated
TEST_CASE("simple", "[json_balance_ack]") {
  auto message = R"({)"
                 R"("status":"ok",)"
                 R"("data":{)"
                 R"("id":71463752,)"
                 R"("type":"spot",)"
                 R"("state":"working",)"
                 R"("list":[{)"
                 R"("currency":"ioi",)"
                 R"("type":"trade",)"
                 R"("balance":"0",)"
                 R"("available":"0",)"
                 R"("debt":"0",)"
                 R"("seq-num":"0")"
                 R"(},{)"
                 R"("currency":"ioi",)"
                 R"("type":"frozen",)"
                 R"("balance":"0",)"
                 R"("debt":"0",)"
                 R"("seq-num":"0")"
                 R"(},{)"
                 R"("currency":"wild",)"
                 R"("type":"trade",)"
                 R"("balance":"0",)"
                 R"("available":"0",)"
                 R"("debt":"0",)"
                 R"("seq-num":"0")"
                 R"(},{)"
                 R"("currency":"wild",)"
                 R"("type":"frozen",)"
                 R"("balance":"0",)"
                 R"("debt":"0",)"
                 R"("seq-num":"0")"
                 R"(},{)"
                 R"("currency":"bfc",)"
                 R"("type":"trade",)"
                 R"("balance":"0",)"
                 R"("available":"0",)"
                 R"("debt":"0",)"
                 R"("seq-num":"0")"
                 R"(},{)"
                 R"("currency":"bfc",)"
                 R"("type":"frozen",)"
                 R"("balance":"0",)"
                 R"("debt":"0",)"
                 R"("seq-num":"0")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) { CHECK(obj.status == protocol::json::Status::OK); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
