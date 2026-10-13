/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/htx/gateway/account.hpp"

#include "roq/utils/safe_cast.hpp"

namespace roq {
namespace htx {
namespace gateway {

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name, roq::io::web::URI const &uri)
    : name{name}, crypto_{config.get_api_key(name), config.get_secret(name), uri.get_host()} {
}

std::string_view Account::create_query(TraceInfo const &trace_info, web::http::Method method, std::string_view const &path) {
  return crypto_.create_query(method, path, utils::safe_cast(trace_info.origin_create_time_utc));
}

std::string_view Account::create_ws_auth(TraceInfo const &trace_info, std::string_view const &path) {
  return crypto_.create_ws_auth(path, utils::safe_cast(trace_info.origin_create_time_utc));
}

}  // namespace gateway
}  // namespace htx
}  // namespace roq
