/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/huobi/application.h"

using namespace std::literals;

namespace {
static const auto DESCRIPTION = "Roq Huobi Gateway"sv;
}  // namespace

int main(int argc, char **argv) {
  return roq::huobi::Application(
             argc, argv, DESCRIPTION, ROQ_BUILD_VERSION, ROQ_BUILD_TYPE, ROQ_GIT_DESCRIBE_HASH)
      .run();
}
