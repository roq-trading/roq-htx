/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/huobi/application.h"

#include "roq/huobi/config.h"
#include "roq/huobi/flags.h"
#include "roq/huobi/gateway.h"

using namespace roq::literals;

namespace roq {
namespace huobi {

int Application::main(int, char **) {
  log::info(R"(Parse config_file="{}")"_sv, Flags::config_file());
  Config config(Flags::config_file(), Flags::secrets_file());
  log::info<1>("config={}"_sv, config);
  log::info("Starting the gateway"_sv);
  roq::server::Trading<Gateway>(ROQ_PACKAGE_NAME, config, server::RequestIdType::SEQUENTIAL, config)
      .dispatch();
  return EXIT_SUCCESS;
}

}  // namespace huobi
}  // namespace roq
