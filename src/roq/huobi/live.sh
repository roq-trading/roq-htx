#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
	PREFIX="gdb --args"
else
	PREFIX=
fi

NAME="huobi"

CONFIG_FILE="$CWD/config/$NAME.toml"

URI="api.huobi.pro"

REST_URI="https://$URI"
WS_URI="wss://$URI/ws"
WS_MBP_URI="wss://$URI/feed"
WS_ORDER_URI="wss://$URI/ws/v2"

$PREFIX ./roq-huobi \
	--name "$NAME" \
	--config_file "$CONFIG_FILE" \
	--client_listen_address $CWD/$NAME.sock \
	--metrics_listen_address $CWD/${NAME}_metrics.sock \
	--rest_uri "$REST_URI" \
	--ws_market_uri "$WS_URI" \
	--ws_mbp_uri "$WS_MBP_URI" \
	--ws_order_uri "$WS_ORDER_URI" \
	$@
