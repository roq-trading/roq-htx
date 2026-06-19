/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/htx/gateway/order_entry.hpp"

#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/htx/protocol/json/encoder.hpp"
#include "roq/htx/protocol/json/map.hpp"
#include "roq/htx/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace htx {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const NAME = "om"sv;

auto const SUPPORTS = Mask{
    SupportType::CREATE_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER,
    SupportType::ORDER_ACK,
    SupportType::FUNDS,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.rest.uri;
  auto config = web::rest::Client::Config{
      // connection
      .interface = {},
      .proxy = settings.rest.proxy,
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = {},
      .disconnect_on_idle_timeout = {},
      .connection = web::http::Connection::KEEP_ALIVE,
      // request
      .allow_pipelining = true,
      .request_timeout = settings.rest.request_timeout,
      // response
      .suspend_on_retry_after = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .ping_frequency = settings.rest.ping_freq,
      .ping_path = settings.rest.ping_path,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::rest::Client::create(handler, context, config);
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

OrderEntry::OrderEntry(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.name)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .accounts = create_metrics(shared.settings, name_, "accounts"sv),
          .accounts_ack = create_metrics(shared.settings, name_, "accounts_ack"sv),
          .balance = create_metrics(shared.settings, name_, "balance"sv),
          .balance_ack = create_metrics(shared.settings, name_, "balance_ack"sv),
          .open_orders = create_metrics(shared.settings, name_, "open_orders"sv),
          .open_orders_ack = create_metrics(shared.settings, name_, "open_orders_ack"sv),
          .place_order = create_metrics(shared.settings, name_, "place_order"sv),
          .place_order_ack = create_metrics(shared.settings, name_, "place_order_ack"sv),
          .cancel_order = create_metrics(shared.settings, name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(shared.settings, name_, "cancel_order_ack"sv),
          .cancel_all_orders = create_metrics(shared.settings, name_, "cancel_all_orders"sv),
          .cancel_all_orders_ack = create_metrics(shared.settings, name_, "cancel_all_orders_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      account_{account}, shared_{shared}, download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }} {
}

void OrderEntry::operator()(Event<Start> const &) {
  (*connection_).start();
}

void OrderEntry::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void OrderEntry::operator()(Event<Timer> const &event) {
  (*connection_).refresh(event.value.now);
}

void OrderEntry::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.accounts, metrics::Type::PROFILE)
      .write(profile_.accounts_ack, metrics::Type::PROFILE)
      .write(profile_.balance, metrics::Type::PROFILE)
      .write(profile_.balance_ack, metrics::Type::PROFILE)
      .write(profile_.open_orders, metrics::Type::PROFILE)
      .write(profile_.open_orders_ack, metrics::Type::PROFILE)
      .write(profile_.place_order, metrics::Type::PROFILE)
      .write(profile_.place_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_order, metrics::Type::PROFILE)
      .write(profile_.cancel_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_all_orders, metrics::Type::PROFILE)
      .write(profile_.cancel_all_orders_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

uint16_t OrderEntry::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  place_order(event, order, ref_data, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &,
    server::oms::Order const &,
    server::oms::RefData const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  cancel_order(event, order, ref_data, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  cancel_all_orders(event, request_id);
  return stream_id_;
}

void OrderEntry::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    download_.begin();
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading()) {
    download_.reset();
  }
  account_id_ = {};  // ???
}

void OrderEntry::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(shared_.dispatcher, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
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
  create_trace_and_dispatch(shared_.dispatcher, trace_info, stream_status);
}

uint32_t OrderEntry::download(State state) {
  switch (state) {
    using enum State;
    case UNDEFINED:
      assert(false);
      break;
    case ACCOUNTS:
      (*this)(ConnectionStatus::DOWNLOADING, "accounts"sv);
      accounts();
      return 1;
    case BALANCE:
      (*this)(ConnectionStatus::DOWNLOADING, "balance"sv);
      balance();
      return 1;
    case OPEN_ORDERS:
      (*this)(ConnectionStatus::DOWNLOADING, "open-orders"sv);
      open_orders();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

// accounts

void OrderEntry::accounts() {
  profile_.accounts([&]() {
    auto now_utc = clock::get_realtime<std::chrono::seconds>();
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.accounts;
    auto query = account_.create_query(method, path, now_utc);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      accounts_ack(event);
    };
    (*connection_)("orders"sv, request, callback);
  });
}

void OrderEntry::accounts_ack(Trace<web::rest::Response> const &event) {
  auto const STATE = State::ACCOUNTS;
  profile_.accounts_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      if (download_.downloading()) {
        download_.retry(STATE);
      }
    };
    auto handle_success = [&](auto &body) {
      protocol::json::AccountsAck accounts_ack{body, decode_buffer_};
      if (accounts_ack.status == protocol::json::Status::OK) {
        Trace event_2{event, accounts_ack};
        (*this)(event_2);
        download_.check_relaxed(STATE);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(accounts_ack.err_code), accounts_ack.err_msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::AccountsAck> const &event) {
  auto &[trace_info, accounts_ack] = event;
  log::info<2>("accounts_ack={}"sv, accounts_ack);
  for (auto &item : accounts_ack.data) {
    if (utils::update(account_id_, item.id)) {
      log::warn("DEBUG account_id={}"sv, account_id_);
    }
  }
}

// balance

void OrderEntry::balance() {
  profile_.balance([&]() {
    auto now_utc = clock::get_realtime<std::chrono::seconds>();
    auto method = web::http::Method::GET;
    auto path = fmt::format("{}/{}/balance"sv, shared_.api.order_management.accounts, account_id_);
    auto query = account_.create_query(method, path, now_utc);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      balance_ack(event);
    };
    (*connection_)("orders"sv, request, callback);
  });
}

void OrderEntry::balance_ack(Trace<web::rest::Response> const &event) {
  auto const STATE = State::BALANCE;
  profile_.balance_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      if (download_.downloading()) {
        download_.retry(STATE);
      }
    };
    auto handle_success = [&](auto &body) {
      protocol::json::BalanceAck balance_ack{body, decode_buffer_};
      if (balance_ack.status == protocol::json::Status::OK) {
        Trace event_2{event, balance_ack};
        (*this)(event_2);
        download_.check_relaxed(STATE);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(balance_ack.err_code), balance_ack.err_msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::BalanceAck> const &event) {
  auto &[trace_info, balance_ack] = event;
  log::info<2>("balance_ack={}"sv, balance_ack);
  auto &data = balance_ack.data;
  auto external_account = fmt::format("{}"sv, data.id);
  for (auto &item : data.list) {
    auto funds_update = FundsUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .currency = item.currency,
        .margin_mode = {},
        .balance = item.balance,
        .hold = NaN,
        .borrowed = NaN,
        .unrealized_pnl = NaN,
        .external_account = external_account,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = {},
        .exchange_sequence = utils::safe_cast(item.seq_num),
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(shared_.dispatcher, trace_info, funds_update, true);
  }
}

// open-orders

void OrderEntry::open_orders() {
  profile_.open_orders([&]() {
    auto now_utc = clock::get_realtime<std::chrono::seconds>();
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.open_orders;
    auto query = account_.create_query(method, path, now_utc);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      open_orders_ack(event);
    };
    (*connection_)("orders"sv, request, callback);
  });
}

void OrderEntry::open_orders_ack(Trace<web::rest::Response> const &event) {
  auto const STATE = State::OPEN_ORDERS;
  profile_.open_orders_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      if (download_.downloading()) {
        download_.retry(STATE);
      }
    };
    auto handle_success = [&](auto &body) {
      protocol::json::OpenOrdersAck open_orders_ack{body, decode_buffer_};
      if (open_orders_ack.status == protocol::json::Status::OK) {
        Trace event_2{event, open_orders_ack};
        (*this)(event_2);
        download_.check_relaxed(STATE);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(open_orders_ack.err_code), open_orders_ack.err_msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::OpenOrdersAck> const &event) {
  auto &[trace_info, open_orders_ack] = event;
  log::info<2>("open_orders_ack={}"sv, open_orders_ack);
  for (auto &item : open_orders_ack.data) {
    auto external_account = fmt::format("{}"sv, item.account_id);
    auto external_order_id = fmt::format("{}"sv, item.id);
    auto remaining_quantity = item.amount - item.filled_amount;
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .side = map(item.type),
        .position_effect = {},
        .margin_mode = {},
        .max_show_quantity = NaN,
        .order_type = map(item.type),
        .time_in_force = map(item.type),
        .execution_instructions = map(item.type),
        .create_time_utc = item.created_at,
        .update_time_utc = item.created_at,
        .external_account = external_account,
        .external_order_id = external_order_id,
        .client_order_id = item.client_order_id,
        .order_status = map(item.state),
        .error = {},
        .text = {},
        .quantity = item.amount,
        .price = item.price,
        .stop_price = NaN,
        .leverage = NaN,
        .remaining_quantity = remaining_quantity,
        .traded_quantity = item.filled_amount,
        .average_traded_price = NaN,
        .last_traded_quantity = NaN,
        .last_traded_price = NaN,
        .last_liquidity = {},
        .routing_id = {},
        .max_request_version = {},
        .max_response_version = {},
        .max_accepted_version = {},
        .update_type = UpdateType::SNAPSHOT,
        .sending_time_utc = item.created_at,
    };
    create_trace_and_dispatch(shared_.dispatcher, trace_info, order_update, stream_id_);
  }
}

// place-order

void OrderEntry::place_order(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  profile_.place_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, create_order] = event;
    auto now_utc = clock::get_realtime<std::chrono::seconds>();
    auto method = web::http::Method::POST;
    auto path = shared_.api.order_management.place_order;
    auto query = account_.create_query(method, path, now_utc);
    auto body = protocol::json::Encoder::place_order(encode_buffer_, create_order, order, ref_data, request_id, account_id_);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = {},
        .body = body,
        .quality_of_service = {},
    };
    auto callback = [this, user_id = message_info.source, order_id = create_order.order_id]([[maybe_unused]] auto &request_id, auto &response) {
      auto version = 1;
      TraceInfo trace_info;
      Trace event{trace_info, response};
      place_order_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::place_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.place_order_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::debug(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::CREATE_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .external_order_id = {},
          .client_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      create_trace_and_dispatch(shared_.dispatcher, trace_info, response, stream_id_, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::PlaceOrderAck place_order_ack{body, decode_buffer_};
      if (place_order_ack.status == protocol::json::Status::OK) {
        Trace event_2{event, place_order_ack};
        (*this)(event_2, user_id, order_id, version);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(place_order_ack.err_code), place_order_ack.err_msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::PlaceOrderAck> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, place_order_ack] = event;
  log::info<2>("place_order_ack={}"sv, place_order_ack);
  // XXX FIXME TODO place_order_ack.data is the exchange order id !!!
  auto response = server::oms::Response{
      .request_type = RequestType::CREATE_ORDER,
      .origin = Origin::EXCHANGE,
      .request_status = RequestStatus::ACCEPTED,
      .error = {},
      .text = {},
      .version = version,
      .request_id = place_order_ack.client_order_id,
      .external_order_id = {},
      .client_order_id = place_order_ack.client_order_id,
      .quantity = NaN,
      .price = NaN,
  };
  create_trace_and_dispatch(shared_.dispatcher, trace_info, response, stream_id_, user_id, order_id);
}

// cancel-order

void OrderEntry::cancel_order(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, cancel_order] = event;
    auto helper = [&](auto &path, auto &body) {
      auto now_utc = clock::get_realtime<std::chrono::seconds>();
      auto method = web::http::Method::POST;
      auto query = account_.create_query(method, path, now_utc);
      auto request = web::rest::Request{
          .method = method,
          .path = path,
          .query = query,
          .accept = web::http::Accept::APPLICATION_JSON,
          .content_type = web::http::ContentType::APPLICATION_JSON,
          .headers = {},
          .body = body,
          .quality_of_service = {},
      };
      auto callback = [this, user_id = message_info.source, order_id = cancel_order.order_id]([[maybe_unused]] auto &request_id, auto &response) {
        auto version = 1;
        TraceInfo trace_info;
        Trace event{trace_info, response};
        cancel_order_ack(event, user_id, order_id, version);
      };
      (*connection_)(request_id, request, callback);
    };
    auto path = shared_.api.order_management.cancel_order;
    if (std::empty(order.external_order_id)) {
      auto path_2 = fmt::format("{}/submitCancelClientOrder"sv, path);
      auto body = protocol::json::Encoder::cancel_client_order(encode_buffer_, cancel_order, order, ref_data, request_id, previous_request_id);
      helper(path_2, body);
    } else {
      auto path_2 = fmt::format("{}/{}/submitcancel"sv, path, order.external_order_id);
      auto body = protocol::json::Encoder::cancel_order(encode_buffer_, cancel_order, order, ref_data, request_id, previous_request_id);
      helper(path_2, body);
    }
  });
}

void OrderEntry::cancel_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.cancel_order_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::debug(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::CREATE_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .external_order_id = {},
          .client_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      create_trace_and_dispatch(shared_.dispatcher, trace_info, response, stream_id_, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::CancelOrderAck cancel_order_ack{body, decode_buffer_};
      if (cancel_order_ack.status == protocol::json::Status::OK) {
        Trace event_2{event, cancel_order_ack};
        (*this)(event_2, user_id, order_id, version);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(cancel_order_ack.err_code), cancel_order_ack.err_msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::CancelOrderAck> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, cancel_order_ack] = event;
  log::info<2>("cancel_order_ack={}"sv, cancel_order_ack);
  // XXX FIXME TODO cancel_order_ack.data is the exchange order id !!!
  auto response = server::oms::Response{
      .request_type = RequestType::CREATE_ORDER,
      .origin = Origin::EXCHANGE,
      .request_status = RequestStatus::ACCEPTED,
      .error = {},
      .text = {},
      .version = version,
      .request_id = {},
      .external_order_id = {},
      .client_order_id = {},
      .quantity = NaN,
      .price = NaN,
  };
  create_trace_and_dispatch(shared_.dispatcher, trace_info, response, stream_id_, user_id, order_id);
}

// cancel-all-orders

void OrderEntry::cancel_all_orders(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  profile_.cancel_all_orders([&]() {
    if (!ready()) [[unlikely]] {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, cancel_all_orders] = event;
    auto now_utc = clock::get_realtime<std::chrono::seconds>();
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.cancel_all_orders;
    auto query = account_.create_query(method, path, now_utc);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this](auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      cancel_all_orders_ack(event, request_id);
    };
    (*connection_)(request_id, request, callback);
    auto cancel_all_orders_ack = CancelAllOrdersAck{
        .stream_id = stream_id_,
        .account = account_.name,
        .order_id = cancel_all_orders.order_id,
        .exchange = cancel_all_orders.exchange,
        .symbol = cancel_all_orders.symbol,
        .side = cancel_all_orders.side,
        .origin = Origin::GATEWAY,
        .request_status = RequestStatus::FORWARDED,
        .error = {},
        .text = {},
        .request_id = request_id,
        .external_account = {},
        .number_of_affected_orders = {},
        .round_trip_latency = {},
        .user = {},
        .strategy_id = cancel_all_orders.strategy_id,
    };
    TraceInfo trace_info{event};
    create_trace_and_dispatch(shared_.dispatcher, trace_info, cancel_all_orders_ack);
  });
}

void OrderEntry::cancel_all_orders_ack(Trace<web::rest::Response> const &event, std::string_view const &request_id) {
  profile_.cancel_all_orders_ack([&]() {
    auto send_ack = [&](auto origin, auto status, Error error, std::string_view const &text) {
      auto cancel_all_orders_ack = CancelAllOrdersAck{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = {},
          .exchange = {},
          .symbol = {},
          .side = {},
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .request_id = request_id,
          .external_account = {},
          .number_of_affected_orders = {},
          .round_trip_latency = {},
          .user = {},
          .strategy_id = {},
      };
      create_trace_and_dispatch(shared_.dispatcher, event, cancel_all_orders_ack);
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::debug(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      send_ack(origin, RequestStatus::REJECTED, error, text);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::CancelAllOrdersAck cancel_all_orders_ack{body, decode_buffer_};
      // XXX FIXME TODO ret_code ???
      Trace event_2{event, cancel_all_orders_ack};
      (*this)(event_2);
      send_ack(Origin::EXCHANGE, RequestStatus::ACCEPTED, {}, {});
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::CancelAllOrdersAck> const &event) {
  auto &[trace_info, cancel_all_orders_ack] = event;
  log::info<2>("cancel_all_orders_ack={}"sv, cancel_all_orders_ack);
}

// helpers

void OrderEntry::process_response(web::rest::Response const &response, auto error_handler, auto success_handler) {
  try {
    auto [status, category, body] = response.result();
    switch (category) {
      using enum web::http::Category;
      case UNKNOWN:
      case INFORMATIONAL_RESPONSE:
        response.expect(web::http::Status::OK);  // throws
        break;
      case SUCCESS:
        success_handler(body);
        break;
      case REDIRECTION:
        log::fatal("Unexpected: URL is being redirected"sv);
      case CLIENT_ERROR:
        switch (status) {
          using enum web::http::Status;
          case FORBIDDEN:            // 403
          case I_AM_A_TEAPOT:        // 418
          case TOO_MANY_REQUESTS: {  // 429
            auto text = fmt::format("{}"sv, status);
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::REQUEST_RATE_LIMIT_REACHED, text);
            break;
          }
          case CONFLICT:  // 409
            assert(false);
            [[fallthrough]];
          default:
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, ""sv);
        }
        break;
      case SERVER_ERROR: {
        auto text = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, text);
        break;
      }
    }
  } catch (server::oms::Exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(e.origin, e.status, e.error, e.what());
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), e.what());
  } catch (std::exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, e.what());
  }
}

}  // namespace gateway
}  // namespace htx
}  // namespace roq
