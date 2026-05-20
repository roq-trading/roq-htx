/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/htx/gateway/api.hpp"

using namespace std::literals;

namespace roq {
namespace htx {
namespace gateway {

// === IMPLEMENTATION ===

API API::create(Settings const &) {
  return {
      .market_data{
          .get_market_status = "/v2/market-status"sv,
          .get_currencies = "/v1/common/currencys"sv,
          .get_symbols = "/v1/common/symbols"sv,
          .mbp_depth_theme = "mbp.20"sv,  // note! 150 is throttled
      },
      .order_management{
          .accounts = "/v1/account/accounts"sv,
          .open_orders = "/v1/order/openOrders"sv,
          .place_order = "/v1/order/orders/place"sv,
          .cancel_order = "/v1/order/orders"sv,
          .cancel_client_order = "/v1/order/orders/submitCancelClientOrder"sv,
          .cancel_all_orders = "/v1/order/cancelAllOrders"sv,
      },
  };
}

}  // namespace gateway
}  // namespace htx
}  // namespace roq
