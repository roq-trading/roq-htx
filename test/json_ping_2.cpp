/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Ping;

TEST_CASE("simple", "[json_ping]") {
  auto message = R"({)"
                 R"("ping":1764300452378)"
                 R"(})";
  auto helper = [](value_type const &obj) { CHECK(obj.ping == 1764300452378ms); };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
