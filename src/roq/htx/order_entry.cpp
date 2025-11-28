/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/htx/order_entry.hpp"

#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/htx/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace htx {

// === CONSTANTS ===

namespace {
auto const NAME = "om"sv;

auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
    SupportType::CREATE_ORDER,
    SupportType::CANCEL_ORDER,
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
          .open_orders = create_metrics(shared.settings, name_, "open_orders"sv),
          .open_orders_ack = create_metrics(shared.settings, name_, "open_orders_ack"sv),
          .new_order = create_metrics(shared.settings, name_, "new_order"sv),
          .new_order_ack = create_metrics(shared.settings, name_, "new_order_ack"sv),
          .cancel_order = create_metrics(shared.settings, name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(shared.settings, name_, "cancel_order_ack"sv),
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
      .write(profile_.open_orders, metrics::Type::PROFILE)
      .write(profile_.open_orders_ack, metrics::Type::PROFILE)
      .write(profile_.new_order, metrics::Type::PROFILE)
      .write(profile_.new_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_order, metrics::Type::PROFILE)
      .write(profile_.cancel_order_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

uint16_t OrderEntry::operator()(Event<CreateOrder> const &, server::oms::Order const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &,
    server::oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &,
    server::oms::Order const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t OrderEntry::operator()(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"CancelAllOrders"sv};
  return stream_id_;
}

void OrderEntry::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading()) {
    download_.reset();
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.name,
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    using enum OrderEntryState;
    case UNDEFINED:
      assert(false);
      break;
    case ACCOUNTS:
      accounts();
      return 1;
    case OPEN_ORDERS:
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
    log::warn("DEBUG request={}"sv, request);
    auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      accounts_ack(event);
    };
    (*connection_)("orders"sv, request, callback);
  });
}

void OrderEntry::accounts_ack(Trace<web::rest::Response> const &event) {
  auto const STATE = OrderEntryState::ACCOUNTS;
  profile_.accounts_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      if (download_.downloading()) {
        download_.retry(STATE);
      }
    };
    auto handle_success = [&](auto &body) {
      log::warn("DEBUG {}"sv, body);
      json::Accounts accounts{body, decode_buffer_};
      if (accounts.status == json::Status::OK) {
        Trace event_2{event, accounts};
        (*this)(event_2);
        download_.check_relaxed(STATE);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(accounts.err_code), accounts.err_msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::Accounts> const &event) {
  auto &[trace_info, accounts] = event;
  log::info<2>("accounts={}"sv, accounts);
  // XXX FIXME TODO
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
    log::warn("DEBUG request={}"sv, request);
    auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      open_orders_ack(event);
    };
    (*connection_)("orders"sv, request, callback);
  });
}

void OrderEntry::open_orders_ack(Trace<web::rest::Response> const &event) {
  auto const STATE = OrderEntryState::OPEN_ORDERS;
  profile_.open_orders_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      if (download_.downloading()) {
        download_.retry(STATE);
      }
    };
    auto handle_success = [&](auto &body) {
      log::warn("DEBUG {}"sv, body);
      json::OpenOrders open_orders{body, decode_buffer_};
      if (open_orders.status == json::Status::OK) {
        Trace event_2{event, open_orders};
        (*this)(event_2);
        download_.check_relaxed(STATE);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(open_orders.err_code), open_orders.err_msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::OpenOrders> const &event) {
  auto &[trace_info, open_orders] = event;
  log::info<2>("open_orders={}"sv, open_orders);
  // XXX FIXME TODO
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

template <typename... Args>
void OrderEntry::operator()(Trace<server::oms::Response> const &event, uint8_t user_id, uint64_t order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(user_id, order_id, stream_id_, trace_info, response, std::forward<Args>(args)..., []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
  }
}

void OrderEntry::operator()(Trace<server::oms::OrderUpdate> const &event, std::string_view const &client_order_id) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

}  // namespace htx
}  // namespace roq
