set(TARGET_NAME "${PROJECT_NAME}-test")

set(SOURCES
    json_bbo.cpp
    json_currencies.cpp
    json_detail.cpp
    json_market_status.cpp
    json_mbp.cpp
    json_mbp_snapshot.cpp
    json_symbols.cpp
    json_ticker.cpp
    json_trade.cpp
    json_utils.cpp
    main.cpp)

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}")

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries(
  "${TARGET_NAME}"
  "${PROJECT_NAME}-json"
  roq-server::roq-server
  roq-core::roq-core
  roq-io::roq-io
  fmt::fmt
  Catch2::Catch2
  ${RT_LIBRARIES})

target_compile_features("${TARGET_NAME}" PUBLIC cxx_std_20)

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

# run

add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
