/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/huobi/application.hpp"

#include "roq/huobi/config.hpp"
#include "roq/huobi/gateway.hpp"

using namespace std::literals;

namespace roq {
namespace huobi {

int Application::main(int, char **) {
  Config config;
  log::info<1>("config={}"sv, config);
  auto context = server::create_io_context();
  server::Settings settings{
      .package_name = ROQ_PACKAGE_NAME,
      .build_number = ROQ_BUILD_NUMBER,
      .api = {},
      .type = server::Type::ORDER_MANAGEMENT,
  };
  server::Trading<Gateway>(settings, config, *context).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace huobi
}  // namespace roq
