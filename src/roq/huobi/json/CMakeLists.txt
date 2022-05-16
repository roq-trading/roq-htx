set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES parser.cpp)

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

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/huobi/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "hpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT
  AUTOGEN_SOURCES
  NAMESPACE
  "roq/huobi/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "cpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}" ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library("${TARGET_NAME}" STATIC ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" autogen-json-headers)

target_link_libraries("${TARGET_NAME}" roq-core::roq-core roq-logging::roq-logging fmt::fmt)
