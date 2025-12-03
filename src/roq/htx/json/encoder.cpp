/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/htx/json/encoder.hpp"

#include "roq/decimal.hpp"

#include "roq/htx/json/map.hpp"
#include "roq/htx/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace htx {
namespace json {

// === IMPLEMENTATION ===

// self-match-prevent  int false self match prevent. 0: no, means allowing self-trading; 1: yes, means not allowing self-trading   0
// stop-price  string  false Trigger price of stop limit order   NA
// operator  string  false operation charactor of stop price

std::string_view Encoder::place_order(
    std::string &buffer, CreateOrder const &create_order, server::oms::Order const &order, std::string_view const &request_id, int64_t account_id) {
  buffer.clear();
  auto type = map(create_order.side, create_order.order_type, create_order.time_in_force, create_order.execution_instructions).template get<json::OrderType>();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("account-id":"{}",)"
      R"("symbol":"{}",)"
      R"("client-order-id":"{}",)"
      R"("type":"{}",)"
      R"("amount":"{}")"sv,
      account_id,
      create_order.symbol,
      request_id,
      type.as_raw_text(),
      Decimal{create_order.quantity, order.quantity_precision.precision});
  if (!std::isnan(create_order.price)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"price":"{}")"sv, Decimal{create_order.price, order.price_precision.precision});
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"(,"source":"spot-api")"
      R"(,"self-match-prevent":0)"
      R"(}})"sv);
  return buffer;
}

std::string_view Encoder::cancel_order(
    std::string &buffer,
    CancelOrder const &,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  assert(!std::empty(order.external_order_id));
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("order-id":"{}",)"
      R"("symbol":"{}")"
      R"(}})"sv,
      order.external_order_id,
      order.symbol);
  return buffer;
}

std::string_view Encoder::cancel_client_order(
    std::string &buffer,
    CancelOrder const &,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("client-order-id":"{}")"
      R"(}})"sv,
      order.client_order_id);
  return buffer;
}

}  // namespace json
}  // namespace htx
}  // namespace roq
