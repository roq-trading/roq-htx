/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/huobi/application.hpp"

#include "roq/huobi/config.hpp"
#include "roq/huobi/gateway.hpp"
#include "roq/huobi/settings.hpp"

using namespace std::literals;

namespace roq {
namespace huobi {

// === CONSTANTS ===

namespace {
auto const TYPE = server::Type::ORDER_MANAGEMENT;
}

// === IMPLEMENTATION ===

int Application::main(args::Parser const &) {
  Settings settings{TYPE};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace huobi
}  // namespace roq
