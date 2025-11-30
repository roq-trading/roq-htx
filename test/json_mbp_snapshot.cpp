/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/htx/json/parser.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

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
  core::json::BufferStack buffers{8192, 1};
  // simple
  json::MBPSnapshot obj{message, buffers};
  CHECK(obj.id == 3000002);
  // parser
  struct Handler final : public json::Parser::Handler {
    void operator()(Trace<json::Req> const &) override { FAIL(); }
    void operator()(Trace<json::Ping> const &) override { FAIL(); }
    void operator()(Trace<json::Ping2> const &) override { FAIL(); }
    void operator()(Trace<json::Error> const &) override { FAIL(); }
    void operator()(Trace<json::Error2> const &) override { FAIL(); }
    void operator()(Trace<json::Sub> const &) override { FAIL(); }
    void operator()(Trace<json::Subbed> const &) override { FAIL(); }
    void operator()(Trace<json::BBO> const &) override { FAIL(); }
    void operator()(Trace<json::Trade> const &) override { FAIL(); }
    void operator()(Trace<json::Detail> const &) override { FAIL(); }
    void operator()(Trace<json::Ticker> const &) override { FAIL(); }
    void operator()(Trace<json::MBP> const &) override { FAIL(); }
    void operator()(Trace<json::MBPSnapshot> const &event) override {
      found = true;
      auto &[trace_info, mbp_snapshot] = event;
      CHECK(mbp_snapshot.id == 3000002);
    }

    bool found = false;
  } handler;
  auto res = json::Parser::dispatch(handler, message, buffers, {}, false);
  CHECK(res == true);
  CHECK(handler.found == true);
}
