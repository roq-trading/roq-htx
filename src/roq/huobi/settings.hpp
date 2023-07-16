/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/compile.h>
#include <fmt/format.h>

#include "roq/server/flags/settings.hpp"

#include "roq/huobi/flags/common.hpp"
#include "roq/huobi/flags/flags.hpp"
#include "roq/huobi/flags/rest.hpp"
#include "roq/huobi/flags/ws.hpp"

namespace roq {
namespace huobi {

struct Settings final : public server::flags::Settings, public flags::Flags {
  explicit Settings(args::Parser const &);

  flags::Common common;
  flags::REST rest;
  flags::WS ws;
};

}  // namespace huobi
}  // namespace roq

template <>
struct fmt::formatter<roq::huobi::Settings> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::huobi::Settings const &value, Context &context) const {
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(common={}, )"
        R"(rest={}, )"
        R"(ws={}, )"
        R"(server={})"
        R"(}})"_cf,
        value.common,
        value.rest,
        value.ws,
        static_cast<roq::server::Settings const &>(value));
  }
};
