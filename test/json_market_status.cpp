/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/htx/json/market_status.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_market_status]") {
  auto message = R"({)"
                 R"("code":200,)"
                 R"("message":"success",)"
                 R"("data":{)"
                 R"("marketStatus":1)"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  json::MarketStatus obj{message, buffers};
  CHECK(obj.message == "success"sv);
  auto &data = obj.data;
  CHECK(data.market_status == 1);
}
