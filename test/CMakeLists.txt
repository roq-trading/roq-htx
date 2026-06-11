include(RoqAutogen)

set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES
    json_accounts_ack.cpp
    json_accounts.cpp
    json_balance_ack.cpp
    json_bbo.cpp
    json_cancel_all_orders_ack.cpp
    json_cancel_order_ack.cpp
    json_clearing.cpp
    json_currencies.cpp
    json_detail.cpp
    json_error_2.cpp
    json_error.cpp
    json_market_status.cpp
    json_mbp.cpp
    json_mbp_snapshot.cpp
    json_open_orders_ack.cpp
    json_orders.cpp
    json_ping.cpp
    json_place_order_ack.cpp
    json_req.cpp
    json_subbed.cpp
    json_sub.cpp
    json_symbols.cpp
    json_ticker.cpp
    json_trade.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-protocol-json roq-server::roq-server Catch2::Catch2)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
