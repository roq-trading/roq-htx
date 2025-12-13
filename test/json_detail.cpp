/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Detail;

TEST_CASE("simple", "[json_detail]") {
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.ch == "market.btcusdt.detail"sv);
    CHECK(obj.ts == 1639672356380ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
