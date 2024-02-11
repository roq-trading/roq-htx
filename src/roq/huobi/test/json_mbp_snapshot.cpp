/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/huobi/json/mbp_snapshot.hpp"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

// note! reduced
TEST_CASE("json_mbp_snapshot_simple", "[json_mbp_snapshot]") {
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
  std::vector<std::byte> buffer(8192);
  [[maybe_unused]] json::MBPSnapshot obj{message, buffer};
}
