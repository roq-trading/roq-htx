/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/htx/api.hpp"

using namespace std::literals;

namespace roq {
namespace htx {

// === IMPLEMENTATION ===

API API::create(Settings const &settings) {
  return {
      .market_data{
          .get_market_status = "/v2/market-status"sv,
          .get_currencies = "/v1/common/currencys"sv,
          .get_symbols = "/v1/common/symbols"sv,
          .mbp_depth_theme = "mbp.20"sv,  // note! 150 is throttled
      },
      .order_management{
          .account_info = "/linear-swap-api/v1/swap_account_info"sv,
          .open_orders = "/v1/order/openOrders"sv,
          .place_order = "/linear-swap-api/v1/swap_order"sv,
          .cancel_order = "/linear-swap-api/v1/swap_cancel"sv,
          .cancel_all_orders = "/linear-swap-api/v1/swap_cancelall"sv,
      },
  };
}

}  // namespace htx
}  // namespace roq
