/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "roq/htx/config.hpp"

#include "roq/htx/tools/crypto.hpp"

namespace roq {
namespace htx {

struct Account final {
  Account(Config const &, std::string_view const &name, roq::io::web::URI const &);

  Account(Account const &) = delete;

  std::string_view get_api_key() const { return crypto_.key; }

  std::string_view create_query(web::http::Method, std::string_view const &path, std::chrono::seconds now_utc);

  std::string_view create_ws_auth(std::string_view const &path, std::chrono::seconds now_utc);

  std::string const name;

 private:
  tools::Crypto crypto_;
};

}  // namespace htx
}  // namespace roq
