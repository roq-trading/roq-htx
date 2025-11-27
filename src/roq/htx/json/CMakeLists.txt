set(TARGET_NAME ${PROJECT_NAME}-json)

set(SOURCES map.cpp parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    bbo.json
    bbo_tick.json
    currencies.json
    currencies.json
    detail.json
    detail_tick.json
    error.json
    event_type.json
    frame.json
    market_status_data.json
    market_status.json
    open_orders.json
    mbp.json
    mbp_snapshot_data.json
    mbp_snapshot.json
    mbp_tick.json
    partition.json
    ping.json
    quotation.json
    side.json
    state.json
    status.json
    subbed.json
    symbols_item.json
    symbols.json
    symbol_status.json
    ticker.json
    ticker_tick.json
    topic.json
    trade.json
    trade_tick_item.json
    trading.json
    trade_tick.json)

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
