/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/huobi/application.hpp"

#include "roq/huobi/config.hpp"
#include "roq/huobi/gateway.hpp"
#include "roq/huobi/settings.hpp"

using namespace std::literals;

namespace roq {
namespace huobi {

// === CONSTANTS ===

namespace {
uint8_t const API = {};
}

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context, API}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace huobi
}  // namespace roq
