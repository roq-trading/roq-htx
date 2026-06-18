/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/htx/gateway/drop_copy.hpp"

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/charconv/to_string.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/htx/protocol/json/map.hpp"
#include "roq/htx/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace htx {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::FUNDS,
    SupportType::ORDER,
    SupportType::TRADE,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.ws.order_uri;
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = {},
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() -> std::string { return {}; });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .req = create_metrics(shared.settings, name_, "req"sv),
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .sub = create_metrics(shared.settings, name_, "sub"sv),
          .outbound_account_info = create_metrics(shared.settings, name_, "outbound_account_info"sv),
          .outbound_account_position = create_metrics(shared.settings, name_, "outbound_account_position"sv),
          .balance_update = create_metrics(shared.settings, name_, "balance_update"sv),
          .execution_report = create_metrics(shared.settings, name_, "execution_report"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared} {
}

bool DropCopy::ready() const {
  return (*connection_).ready();
}

void DropCopy::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopy::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopy::operator()(Event<Timer> const &event) {
  (*connection_).refresh(event.value.now);
}

void DropCopy::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.req, metrics::Type::PROFILE)
      .write(profile_.ping, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.sub, metrics::Type::PROFILE)
      .write(profile_.outbound_account_info, metrics::Type::PROFILE)
      .write(profile_.outbound_account_position, metrics::Type::PROFILE)
      .write(profile_.balance_update, metrics::Type::PROFILE)
      .write(profile_.execution_report, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void DropCopy::operator()(web::socket::Client::Connected const &) {
}

void DropCopy::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
}

void DropCopy::operator()(web::socket::Client::Ready const &) {
  send_login();
  (*this)(ConnectionStatus::LOGIN_SENT);
}

void DropCopy::subscribe() {
  subscribe("accounts.update"sv);
  subscribe("orders#*"sv);
  subscribe("trade.clearing#*"sv);
}

void DropCopy::subscribe(std::string_view const &channel) {
  auto message = fmt::format(
      R"({{)"
      R"("action":"sub",)"
      R"("ch":"{}")"
      R"(}})"sv,
      channel);
  (*connection_).send_text(message);
}

void DropCopy::operator()(web::socket::Client::Close const &) {
}

void DropCopy::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void DropCopy::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
  connection_status_ = connection_status;
  TraceInfo trace_info;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = account_.name,
      .supports = SUPPORTS,
      .transport = Transport::TCP,
      .protocol = Protocol::HTTP,
      .encoding = {Encoding::JSON},
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
      .reason = reason,
      .interface = (*connection_).get_interface(),
      .authority = (*connection_).get_current_authority(),
      .path = (*connection_).get_current_path(),
      .proxy = (*connection_).get_proxy(),
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

void DropCopy::send_pong(std::chrono::milliseconds timestamp) {
  auto message = fmt::format(
      R"({{)"
      R"("action":"pong",)"
      R"("data":{{)"
      R"("ts":{})"
      R"(}})"
      R"(}})"sv,
      timestamp.count());
  // log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::send_login() {
  auto now_utc = clock::get_realtime<std::chrono::seconds>();
  auto message = account_.create_ws_auth("/ws/v2"sv, now_utc);
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::parse(std::string_view const &message) {
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!protocol::json::Parser::dispatch(*this, message, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

void DropCopy::operator()(Trace<protocol::json::Req> const &event) {
  profile_.req([&]() {
    auto &[trace_info, req] = event;
    log::debug("req={}"sv, req);
    if (req.ch == "auth"sv) {
      if (req.code == 200) {
        subscribe();
        (*this)(ConnectionStatus::READY);
      } else {
        log::error("req={}"sv, req);
        (*connection_).close();
      }
    }
  });
}

void DropCopy::operator()(Trace<protocol::json::Ping> const &event) {
  profile_.ping([&]() {
    auto &[trace_info, ping] = event;
    send_pong(ping.data.ts);
  });
}

void DropCopy::operator()(Trace<protocol::json::Ping2> const &event) {
  profile_.ping([&]() {
    auto &[trace_info, ping] = event;
    send_pong(ping.ping);
  });
}

void DropCopy::operator()(Trace<protocol::json::Error> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::error("error={}"sv, error);
  });
}

void DropCopy::operator()(Trace<protocol::json::Error2> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::error("error={}"sv, error);
  });
}

void DropCopy::operator()(Trace<protocol::json::Sub> const &event) {
  profile_.sub([&]() {
    auto &[trace_info, sub] = event;
    log::info("sub={}"sv, sub);
  });
}

void DropCopy::operator()(Trace<protocol::json::Subbed> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<protocol::json::BBO> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<protocol::json::Trade> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<protocol::json::Detail> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<protocol::json::Ticker> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<protocol::json::MBP> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<protocol::json::MBPSnapshot> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<protocol::json::Accounts> const &event) {
  auto &[trace_info, accounts] = event;
  log::info<2>("accounts={}"sv, accounts);
  auto &data = accounts.data;
  auto external_account = fmt::format("{}"sv, data.account_id);
  auto update_type = std::empty(data.change_type) ? UpdateType::SNAPSHOT : UpdateType::INCREMENTAL;
  auto funds_update = FundsUpdate{
      .stream_id = stream_id_,
      .account = account_.name,
      .currency = data.currency,
      .margin_mode = {},
      .balance = data.balance,
      .hold = NaN,
      .borrowed = NaN,
      .unrealized_pnl = NaN,
      .external_account = external_account,
      .update_type = update_type,
      .exchange_time_utc = {},
      .exchange_sequence = utils::safe_cast(data.seq_num),
      .sending_time_utc = data.change_time,
  };
  create_trace_and_dispatch(handler_, trace_info, funds_update, true);
}

void DropCopy::operator()(Trace<protocol::json::Orders> const &event) {
  auto &[trace_info, orders] = event;
  log::info<2>("orders={}"sv, orders);
  auto &data = orders.data;
  auto update_time = [&]() {
    if (data.event_type == protocol::json::EventType::TRADE) {
      return data.trade_time;
    }
    if (data.last_act_time.count()) {
      return data.last_act_time;
    }
    return data.order_create_time;
  }();
  auto external_account = fmt::format("{}"sv, data.account_id);
  auto external_order_id = fmt::format("{}"sv, data.order_id);
  auto last_liquidity = [&]() -> Liquidity {
    if (data.event_type == protocol::json::EventType::TRADE) {
      return data.aggressor ? Liquidity::TAKER : Liquidity::MAKER;
    }
    return {};
  }();
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = shared_.settings.exchange,
      .symbol = data.symbol,
      .side = map(data.type),
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = map(data.type),
      .time_in_force = map(data.type),
      .execution_instructions = map(data.type),
      .create_time_utc = data.order_create_time,
      .update_time_utc = update_time,
      .external_account = external_account,
      .external_order_id = external_order_id,
      .client_order_id = data.client_order_id,
      .order_status = map(data.order_status),
      .error = {},
      .text = {},
      .quantity = data.order_size,
      .price = data.order_price,
      .stop_price = NaN,
      .leverage = NaN,
      .remaining_quantity = data.remain_amt,
      .traded_quantity = data.exec_amt,
      .average_traded_price = NaN,
      .last_traded_quantity = data.trade_volume,
      .last_traded_price = data.trade_price,
      .last_liquidity = last_liquidity,
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = UpdateType::INCREMENTAL,
      .sending_time_utc = data.order_create_time,
  };
  create_trace_and_dispatch(shared_.dispatcher, trace_info, order_update, stream_id_);
}

void DropCopy::operator()(Trace<protocol::json::Clearing> const &event) {
  auto &[trace_info, clearing] = event;
  log::info<2>("clearing={}"sv, clearing);
  auto &data = clearing.data;
  auto external_account = fmt::format("{}"sv, data.account_id);
  auto external_order_id = fmt::format("{}"sv, data.order_id);
  auto liquidity = data.aggressor ? Liquidity::TAKER : Liquidity::MAKER;
  auto fill = Fill{
      .exchange_time_utc = data.trade_time,
      .external_trade_id = {},
      .quantity = data.trade_volume,
      .price = data.trade_price,
      .liquidity = liquidity,
      .commission_amount = data.transact_fee,
      .commission_currency = data.fee_currency,
      .base_amount = NaN,
      .quote_amount = NaN,
      .profit_loss_amount = NaN,
  };
  utils::charconv::to_string(std::back_inserter(fill.external_trade_id), data.trade_id);
  auto trade_update = TradeUpdate{
      .stream_id = stream_id_,
      .account = account_.name,
      .order_id = {},
      .exchange = shared_.settings.exchange,
      .symbol = data.symbol,
      .side = map(data.order_side),
      .position_effect = {},
      .margin_mode = {},
      .quantity_type = {},
      .create_time_utc = data.trade_time,
      .update_time_utc = data.trade_time,
      .external_account = external_account,
      .external_order_id = external_order_id,
      .client_order_id = data.client_order_id,
      .fills = {&fill, 1},
      .routing_id = {},
      .update_type = UpdateType::INCREMENTAL,
      .sending_time_utc = data.trade_time,
      .user = {},
      .strategy_id = {},
  };
  create_trace_and_dispatch(handler_, trace_info, trade_update, true, SOURCE_NONE);
}

}  // namespace gateway
}  // namespace htx
}  // namespace roq
