/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/htx/tools/crypto.hpp"

#include <fmt/format.h>

#include <cassert>

#include "roq/logging.hpp"

#include "roq/utils/codec/base64.hpp"
#include "roq/utils/codec/hex.hpp"
#include "roq/utils/codec/url.hpp"

using namespace std::literals;

namespace roq {
namespace htx {
namespace tools {

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &key, std::string_view const &secret, std::string_view const &hostname) : key{key}, mac_{secret}, hostname_{hostname} {
}

std::string_view Crypto::create_query(web::http::Method method, std::string_view const &path, std::chrono::seconds now_utc) {
  assert(!std::empty(path));
  encode_buffer_.clear();
  std::chrono::sys_days days{std::chrono::duration_cast<std::chrono::days>(now_utc)};
  std::chrono::year_month_day ymd{days};
  auto tmp = std::chrono::time_point_cast<std::chrono::seconds>(days);
  auto tmp2 = std::chrono::duration_cast<std::chrono::seconds>(tmp.time_since_epoch());
  auto tmp3 = now_utc - tmp2;
  std::chrono::hh_mm_ss hms{tmp3};
  fmt::format_to(
      std::back_inserter(encode_buffer_),
      "?AccessKeyId={}&"
      "SignatureMethod=HmacSHA256&"
      "SignatureVersion=2&"
      "Timestamp={:04}-{:02}-{:02}T{:02}%3A{:02}%3A{:02}"sv,
      key,
      static_cast<int>(ymd.year()),
      static_cast<unsigned>(ymd.month()),
      static_cast<unsigned>(ymd.day()),
      hms.hours().count(),
      hms.minutes().count(),
      hms.seconds().count());
  auto tmp4 = std::string_view{encode_buffer_}.substr(1);
  mac_.clear();
  mac_.update(magic_enum::enum_name(method));
  mac_.update("\n"sv);
  mac_.update(hostname_);
  mac_.update("\n"sv);
  mac_.update(path);
  mac_.update("\n"sv);
  mac_.update(tmp4);
  auto digest = mac_.final(digest_);
  std::string signature;
  utils::codec::Base64::encode(signature, digest, false, false);
  std::string buffer_2;
  auto signature_2 = utils::codec::URL::encode(buffer_2, signature);
  fmt::format_to(std::back_inserter(encode_buffer_), "&Signature={}"sv, signature_2);
  return encode_buffer_;
}

std::string_view Crypto::create_ws_auth(std::string_view const &path, std::chrono::seconds now_utc) {
  assert(!std::empty(path));
  encode_buffer_.clear();
  std::chrono::sys_days days{std::chrono::duration_cast<std::chrono::days>(now_utc)};
  std::chrono::year_month_day ymd{days};
  auto tmp = std::chrono::time_point_cast<std::chrono::seconds>(days);
  auto tmp2 = std::chrono::duration_cast<std::chrono::seconds>(tmp.time_since_epoch());
  auto tmp3 = now_utc - tmp2;
  std::chrono::hh_mm_ss hms{tmp3};
  fmt::format_to(
      std::back_inserter(encode_buffer_),
      "accessKey={}&"
      "signatureMethod=HmacSHA256&"
      "signatureVersion=2.1&"
      "timestamp={:04}-{:02}-{:02}T{:02}%3A{:02}%3A{:02}"sv,
      key,
      static_cast<int>(ymd.year()),
      static_cast<unsigned>(ymd.month()),
      static_cast<unsigned>(ymd.day()),
      hms.hours().count(),
      hms.minutes().count(),
      hms.seconds().count());
  log::warn("DEBUG {}"sv, encode_buffer_);
  auto tmp4 = std::string_view{encode_buffer_};
  mac_.clear();
  mac_.update("GET\n"sv);
  mac_.update(hostname_);
  mac_.update("\n"sv);
  mac_.update(path);
  mac_.update("\n"sv);
  mac_.update(tmp4);
  auto digest = mac_.final(digest_);
  std::string signature;
  utils::codec::Base64::encode(signature, digest, false, false);
  encode_buffer_.clear();
  fmt::format_to(
      std::back_inserter(encode_buffer_),
      R"({{)"
      R"("action":"req",)"
      R"("ch":"auth",)"
      R"("params":{{)"
      R"("authType":"api",)"
      R"("accessKey":"{}",)"
      R"("signatureMethod":"HmacSHA256",)"
      R"("signatureVersion":"2.1",)"
      R"("timestamp":"{:04}-{:02}-{:02}T{:02}:{:02}:{:02}",)"
      R"("signature":"{}")"
      R"(}})"
      R"(}})"sv,
      key,
      static_cast<int>(ymd.year()),
      static_cast<unsigned>(ymd.month()),
      static_cast<unsigned>(ymd.day()),
      hms.hours().count(),
      hms.minutes().count(),
      hms.seconds().count(),
      signature);
  return encode_buffer_;
}

}  // namespace tools
}  // namespace htx
}  // namespace roq
