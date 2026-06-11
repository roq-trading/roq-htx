/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::MBPSnapshot;

// note! reduced
TEST_CASE("simple", "[json_mbp_snapshot]") {
  auto message = R"({)"
                 R"("id":"3000002",)"
                 R"("status":"ok",)"
                 R"("ts":1639751391156,)"
                 R"("rep":"market.btcusdt.mbp.20",)"
                 R"("data":{)"
                 R"("seqNum":144524033404,)"
                 R"("bids":[)"
                 R"([46705.1,0.211602],)"
                 R"([46703.6,0.038915])"
                 R"(],)"
                 R"("asks":[)"
                 R"([46705.11,0.196215],)"
                 R"([46709.03,0.008795])"
                 R"(])"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == 3000002);
    CHECK(obj.status == protocol::json::Status::OK);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
