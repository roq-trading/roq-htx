/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/htx/json/parser.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_bbo]") {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.bbo",)"
                 R"("ts":1639669832793,)"
                 R"("tick":{)"
                 R"("seqId":144446084385,)"
                 R"("ask":48660.95,)"
                 R"("askSize":0.001298,)"
                 R"("bid":48660.94,)"
                 R"("bidSize":0.228303,)"
                 R"("quoteTime":1639669832791,)"
                 R"("symbol":"btcusdt")"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  // simple
  json::BBO obj{message, buffers};
  CHECK(obj.ch == "market.btcusdt.bbo"sv);
  // parser
  struct Handler final : public json::Parser::Handler {
    void operator()(Trace<json::Req> const &) override { FAIL(); }
    void operator()(Trace<json::Ping> const &) override { FAIL(); }
    void operator()(Trace<json::Ping2> const &) override { FAIL(); }
    void operator()(Trace<json::Error> const &) override { FAIL(); }
    void operator()(Trace<json::Error2> const &) override { FAIL(); }
    void operator()(Trace<json::Sub> const &) override { FAIL(); }
    void operator()(Trace<json::Subbed> const &) override { FAIL(); }
    void operator()(Trace<json::BBO> const &event) override {
      found = true;
      auto &[trace_info, bbo] = event;
      CHECK(bbo.ch == "market.btcusdt.bbo"sv);
    }
    void operator()(Trace<json::Trade> const &) override { FAIL(); }
    void operator()(Trace<json::Detail> const &) override { FAIL(); }
    void operator()(Trace<json::Ticker> const &) override { FAIL(); }
    void operator()(Trace<json::MBP> const &) override { FAIL(); }
    void operator()(Trace<json::MBPSnapshot> const &) override { FAIL(); }

    bool found = false;
  } handler;
  auto res = json::Parser::dispatch(handler, message, buffers, {}, false);
  CHECK(res == true);
  CHECK(handler.found == true);
}
