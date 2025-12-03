/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Subbed;

TEST_CASE("simple", "[json_subbed]") {
  auto message = R"({)"
                 R"("id":"5000001",)"
                 R"("status":"ok",)"
                 R"("subbed":"market.btcusdt.mbp.20",)"
                 R"("ts":1764298751689)"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == 5000001);
    CHECK(obj.status == json::Status::OK);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
