/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/htx/account.hpp"

namespace roq {
namespace htx {

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name) : name_{name}, key_{config.get_api_key(name_)}, crypto_{config.get_secret(name_)} {
}

std::pair<std::string, std::string> Account::create_signature(std::chrono::nanoseconds now) {
  return crypto_.create_signature(now);
}

}  // namespace htx
}  // namespace roq
