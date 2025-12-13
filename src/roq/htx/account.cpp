/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/htx/account.hpp"

namespace roq {
namespace htx {

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name, roq::io::web::URI const &uri)
    : name{name}, crypto_{config.get_api_key(name), config.get_secret(name), uri.get_host()} {
}

std::string_view Account::create_query(web::http::Method method, std::string_view const &path, std::chrono::seconds now_utc) {
  return crypto_.create_query(method, path, now_utc);
}

std::string_view Account::create_ws_auth(std::string_view const &path, std::chrono::seconds now_utc) {
  return crypto_.create_ws_auth(path, now_utc);
}

}  // namespace htx
}  // namespace roq
