/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/htx/json/parser.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_subbed]") {
  auto message = R"({)"
                 R"("id":"5000001",)"
                 R"("status":"ok",)"
                 R"("subbed":"market.btcusdt.mbp.20",)"
                 R"("ts":1764298751689)"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  // simple
  json::Subbed obj{message, buffers};
  CHECK(obj.id == 5000001);
  // parser
  struct Handler final : public json::Parser::Handler {
    void operator()(Trace<json::Req> const &) override { FAIL(); }
    void operator()(Trace<json::Ping> const &) override { FAIL(); }
    void operator()(Trace<json::Ping2> const &) override { FAIL(); }
    void operator()(Trace<json::Error> const &) override { FAIL(); }
    void operator()(Trace<json::Error2> const &) override { FAIL(); }
    void operator()(Trace<json::Sub> const &) override { FAIL(); }
    void operator()(Trace<json::Subbed> const &event) override {
      found = true;
      auto &[trace_info, subbed] = event;
      CHECK(subbed.id == 5000001);
    }
    void operator()(Trace<json::BBO> const &) override { FAIL(); }
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
