/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::Req;

TEST_CASE("auth_success", "[json_req]") {
  auto message = R"({)"
                 R"("action":"req",)"
                 R"("code":200,)"
                 R"("ch":"auth",)"
                 R"("data":{})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == protocol::json::Action::REQ);
    CHECK(obj.code == 200);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("auth_error", "[json_req]") {
  auto message = R"({)"
                 R"("action":"req",)"
                 R"("code":2001,)"
                 R"("ch":"auth",)"
                 R"("message":"invalid.authType")"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == protocol::json::Action::REQ);
    CHECK(obj.code == 2001);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
