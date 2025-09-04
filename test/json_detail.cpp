/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/huobi/json/detail.hpp"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_detail_simple", "[json_detail]") {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.detail",)"
                 R"("ts":1639672356380,)"
                 R"("tick":{)"
                 R"("id":288812213349,)"
                 R"("low":46641.43,)"
                 R"("high":49500.0,)"
                 R"("open":46879.92,)"
                 R"("close":48484.87,)"
                 R"("vol":5.009502243290115E8,)"
                 R"("amount":10312.405491742371,)"
                 R"("version":288812213349,)"
                 R"("count":795572)"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Detail obj{message, buffer};
}
