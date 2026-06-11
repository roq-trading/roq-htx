/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::BBO;

TEST_CASE("simple", "[json_bbo]") {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.bbo",)"
                 R"("ts":1639669832793,)"
                 R"("tick":{)"
                 R"("seqId":144446084385,)"
                 R"("ask":48660.95,)"
                 R"("askSize":0.001298,)"
                 R"("bid":48660.94,)"
                 R"("bidSize":0.228303,)"
                 R"("quoteTime":1639669832791,)"
                 R"("symbol":"btcusdt")"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.ch == "market.btcusdt.bbo"sv);
    CHECK(obj.ts == 1639669832793ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
