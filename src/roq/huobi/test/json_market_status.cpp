/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/huobi/json/market_status.hpp"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_market_status_simple", "[json_market_status]") {
  auto message = R"({)"
                 R"("code":200,)"
                 R"("message":"success",)"
                 R"("data":{)"
                 R"("marketStatus":1)"
                 R"(})"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::MarketStatus::create(message, buffer);
  CHECK(obj.message == "success"sv);
  auto &data = obj.data;
  CHECK(data.market_status == 1);
}
