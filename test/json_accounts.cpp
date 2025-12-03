/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::htx;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Accounts;

TEST_CASE("simple", "[json_accounts]") {
  auto message = R"({)"
                 R"("action":"push",)"
                 R"("ch":"accounts.update",)"
                 R"("data":{)"
                 R"("currency":"usdc",)"
                 R"("accountId":68824237,)"
                 R"("balance":"98.72432",)"
                 R"("available":"98.72432",)"
                 R"("changeType":null,)"
                 R"("accountType":"trade",)"
                 R"("changeTime":null,)"
                 R"("seqNum":1)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::PUSH);
    CHECK(obj.ch == "accounts.update"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("buy_btcusdt_usdt", "[json_accounts]") {
  auto message = R"({)"
                 R"("action":"push",)"
                 R"("ch":"accounts.update",)"
                 R"("data":{)"
                 R"("currency":"usdt",)"
                 R"("accountId":68824237,)"
                 R"("balance":"78.45797228",)"
                 R"("changeType":"order.match",)"
                 R"("accountType":"trade",)"
                 R"("seqNum":18,)"
                 R"("changeTime":1764754899212)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::PUSH);
    CHECK(obj.ch == "accounts.update"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("buy_btcusdt_btc", "[json_accounts]") {
  auto message = R"({)"
                 R"("action":"push",)"
                 R"("ch":"accounts.update",)"
                 R"("data":{)"
                 R"("currency":"btc",)"
                 R"("accountId":68824237,)"
                 R"("balance":"0.0001096",)"
                 R"("changeType":"order.match",)"
                 R"("accountType":"trade",)"
                 R"("seqNum":4,)"
                 R"("changeTime":1764754899212)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::PUSH);
    CHECK(obj.ch == "accounts.update"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
