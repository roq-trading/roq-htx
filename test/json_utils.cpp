/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/huobi/json/utils.hpp"

using namespace roq;
using namespace roq::huobi;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_utils_extract_symbol", "[json_utils]") {
  CHECK(json::extract_symbol("market.btcusdt.ticker"sv) == "btcusdt"sv);
  CHECK(json::extract_symbol("market.btcusdt.bbo"sv) == "btcusdt"sv);
  CHECK(json::extract_symbol("market.btcusdt.trade.detail"sv) == "btcusdt"sv);
  CHECK(json::extract_symbol("market.btcusdt.detail"sv) == "btcusdt"sv);
  CHECK(json::extract_symbol("market.btcusdt.mbp.20"sv) == "btcusdt"sv);
}

TEST_CASE("json_utils_extract_topic", "[json_utils]") {
  CHECK(json::extract_topic("market.btcusdt.ticker"sv) == "ticker"sv);
  CHECK(json::extract_topic("market.btcusdt.bbo"sv) == "bbo"sv);
  CHECK(json::extract_topic("market.btcusdt.trade.detail"sv) == "trade"sv);
  CHECK(json::extract_topic("market.btcusdt.detail"sv) == "detail"sv);
  CHECK(json::extract_topic("market.btcusdt.mbp.20"sv) == "mbp"sv);
}
