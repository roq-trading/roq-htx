#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="huobi"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-huobi/$CONFIG.toml"

URI="api.huobi.pro"

REST_URI="https://$URI"
WS_MARKET_URI="wss://$URI/ws"
WS_MBP_URI="wss://$URI/feed"
WS_ORDER_URI="wss://$URI/ws/v2"

$PREFIX ./roq-huobi \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --rest_uri "$REST_URI" \
  --ws_market_uri "$WS_MARKET_URI" \
  --ws_mbp_uri "$WS_MBP_URI" \
  --ws_order_uri "$WS_ORDER_URI" \
  $@
