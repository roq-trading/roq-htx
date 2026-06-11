/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::Error;

TEST_CASE("simple", "[json_error]") {
  auto message = R"({)"
                 R"("status":"error",)"
                 R"("ts":1764300263709,)"
                 R"("id":"5000001",)"
                 R"("err-code":"bad-request",)"
                 R"("err-msg":"The coin pair does not currently offer subscription services.")"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.status == protocol::json::Status::ERROR);
    CHECK(obj.ts == 1764300263709ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
