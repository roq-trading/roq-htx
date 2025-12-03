set(TARGET_NAME ${PROJECT_NAME}-json)

set(SOURCES encoder.cpp map.cpp parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    accounts_ack_data_item.json
    accounts_ack.json
    accounts_data.json
    accounts.json
    account_state.json
    account_type.json
    action.json
    balance_ack_data.json
    balance_ack_data_list_item.json
    balance_ack.json
    bbo.json
    bbo_tick.json
    cancel_all_orders_ack.json
    cancel_order_ack.json
    currencies.json
    currencies.json
    detail.json
    detail_tick.json
    direction.json
    error_2.json
    error.json
    event_type.json
    market_status_data.json
    market_status.json
    mbp.json
    mbp_snapshot_data.json
    mbp_snapshot.json
    mbp_tick.json
    open_orders_ack_data_item.json
    open_orders_ack.json
    orders_data.json
    orders.json
    order_status.json
    order_type.json
    partition.json
    ping_2.json
    ping_data.json
    ping.json
    place_order_ack.json
    quotation.json
    req.json
    state.json
    status.json
    subbed.json
    sub.json
    symbols_item.json
    symbols.json
    symbol_status.json
    ticker.json
    ticker_tick.json
    topic.json
    trade.json
    trade_tick_item.json
    trade_tick.json
    trading.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen_cpp(
  OUTPUT
  AUTOGEN_SOURCES
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME} ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library(${TARGET_NAME} OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies(${TARGET_NAME} autogen-json-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-codec::roq-codec roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
