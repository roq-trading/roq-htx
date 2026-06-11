/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::Error2;

TEST_CASE("simple", "[json_error_2]") {
  auto message = R"({)"
                 R"("code":2001,)"
                 R"("message":"invalid.action")"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.code == 2001);
    CHECK(obj.message == "invalid.action"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
