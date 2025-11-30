/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/htx/json/parser.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_trade]") {
  auto message = R"({)"
                 R"("ch":"market.btcusdt.trade.detail",)"
                 R"("ts":1639670532436,)"
                 R"("tick":{)"
                 R"("id":144446879688,)"
                 R"("ts":1639670532434,)"
                 R"("data":[{)"
                 R"("id":144446879688433873170760015,)"
                 R"("ts":1639670532434,)"
                 R"("tradeId":102579820014,)"
                 R"("amount":4.74E-4,)"
                 R"("price":48542.5,)"
                 R"("direction":"sell")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffers{8192, 1};
  // simple
  json::Trade obj{message, buffers};
  CHECK(obj.ch == "market.btcusdt.trade.detail"sv);
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
    void operator()(Trace<json::Trade> const &event) override {
      found = true;
      auto &[trace_info, trade] = event;
      CHECK(trade.ch == "market.btcusdt.trade.detail"sv);
    }
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
