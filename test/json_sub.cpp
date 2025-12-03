/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Sub;

TEST_CASE("success", "[json_sub]") {
  auto message = R"({)"
                 R"("action":"sub",)"
                 R"("code":200,)"
                 R"("ch":"trade.clearing#*",)"
                 R"("data":{})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::SUB);
    CHECK(obj.code == 200);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
