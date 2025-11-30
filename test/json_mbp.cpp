/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/htx/json/parser.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_mbp]") {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.mbp.20",)"
                 R"("ts":1639744309814,)"
                 R"("tick":{)"
                 R"("seqNum":144516551156,)"
                 R"("prevSeqNum":144516551150,)"
                 R"("bids":[)"
                 R"([47231.83,0.957689])"
                 R"(])"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  // simple
  json::MBP obj{message, buffers};
  CHECK(obj.ch == "market.btcusdt.mbp.20"sv);
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
    void operator()(Trace<json::MBP> const &event) override {
      found = true;
      auto &[trace_info, mbp] = event;
      CHECK(mbp.ch == "market.btcusdt.mbp.20"sv);
    }
    void operator()(Trace<json::MBPSnapshot> const &) override { FAIL(); }

    bool found = false;
  } handler;
  auto res = json::Parser::dispatch(handler, message, buffers, {}, false);
  CHECK(res == true);
  CHECK(handler.found == true);
}
