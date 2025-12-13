/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/market_status.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::MarketStatus;

TEST_CASE("simple", "[json_market_status]") {
  auto message = R"({)"
                 R"("code":200,)"
                 R"("message":"success",)"
                 R"("data":{)"
                 R"("marketStatus":1)"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.message == "success"sv);
    auto &data = obj.data;
    CHECK(data.market_status == 1);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
