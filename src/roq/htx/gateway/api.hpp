/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/htx/gateway/settings.hpp"

namespace roq {
namespace htx {
namespace gateway {

struct API final {
  struct {
    std::string_view get_market_status;
    std::string_view get_currencies;
    std::string_view get_symbols;
    std::string_view mbp_depth_theme;
  } market_data;
  struct {
    std::string_view accounts = {};
    std::string_view open_orders = {};
    std::string_view place_order = {};
    std::string_view cancel_order = {};
    std::string_view cancel_client_order = {};
    std::string_view cancel_all_orders = {};
  } order_management;

  // factory
  static API create(Settings const &);
};

}  // namespace gateway
}  // namespace htx
}  // namespace roq
