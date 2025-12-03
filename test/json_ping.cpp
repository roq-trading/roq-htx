/* Copyright (c) 2017-2025, Hans Erik Thrane */

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
                 R"("action":"ping",)"
                 R"("data":{)"
                 R"("ts":1764299053602)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::PING);
    CHECK(obj.data.ts == 1764299053602ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
