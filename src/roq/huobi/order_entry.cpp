/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/huobi/order_entry.h"

#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/metrics/factory.h"

#include "roq/huobi/flags.h"

#include "roq/huobi/json/utils.h"

using namespace std::literals;

namespace roq {
namespace huobi {

namespace {
static const auto NAME = "om"sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
    SupportType::CREATE_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::FUNDS,
};

static const auto ALLOW_PIPELINING = true;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

OrderEntry::OrderEntry(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared)
    : handler_(handler), stream_id_(stream_id),
      name_(fmt::format("{}:{}:{}"sv, stream_id_, NAME, security.get_account())),
      connection_(
          *this,
          context,
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          core::URI(Flags::rest_uri()),
          ROQ_PACKAGE_NAME,
          core::http::Connection::KEEP_ALIVE,
          ALLOW_PIPELINING,
          Flags::rest_request_timeout(),
          Flags::rest_ping_freq(),
          Flags::rest_ping_path()),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .listen_key = create_metrics(name_, "listen_key"sv),
          .listen_key_ack = create_metrics(name_, "listen_key_ack"sv),
          .account = create_metrics(name_, "account"sv),
          .account_ack = create_metrics(name_, "account_ack"sv),
          .new_order = create_metrics(name_, "new_order"sv),
          .new_order_ack = create_metrics(name_, "new_order_ack"sv),
          .cancel_order = create_metrics(name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(name_, "cancel_order_ack"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
      },
      security_(security), shared_(shared),
      download_(Flags::rest_request_timeout(), [this](auto state) { return download(state); }) {
}

void OrderEntry::operator()(const Event<Start> &) {
  connection_.start();
}

void OrderEntry::operator()(const Event<Stop> &) {
  connection_.stop();
}

void OrderEntry::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
  refresh_listen_key();
}

void OrderEntry::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.listen_key, metrics::PROFILE)
      .write(profile_.listen_key_ack, metrics::PROFILE)
      .write(profile_.account, metrics::PROFILE)
      .write(profile_.account_ack, metrics::PROFILE)
      .write(profile_.new_order, metrics::PROFILE)
      .write(profile_.new_order_ack, metrics::PROFILE)
      .write(profile_.cancel_order, metrics::PROFILE)
      .write(profile_.cancel_order_ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

uint16_t OrderEntry::operator()(
    const Event<CreateOrder> &event, const oms::Order &order, const std::string_view &request_id) {
  new_order(event, order, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<ModifyOrder> &,
    const oms::Order &,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  throw oms::NotSupportedException();
}

uint16_t OrderEntry::operator()(
    const Event<CancelOrder> &event,
    const oms::Order &order,
    const std::string_view &request_id,
    const std::string_view &previous_request_id) {
  cancel_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<CancelAllOrders> &, [[maybe_unused]] const std::string_view &request_id) {
  log::fatal("*** CANCEL ALL ORDERS *NOT* SUPPORTED ***"sv);
}

void OrderEntry::operator()(const core::web::Client::Connected &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntry::operator()(const core::web::Client::Disconnected &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void OrderEntry::operator()(const core::web::Client::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::REST,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    case OrderEntryState::UNDEFINED:
      assert(false);
      break;
    case OrderEntryState::LISTEN_KEY:
      get_listen_key();
      return 1;
    case OrderEntryState::ACCOUNT:
      get_account();
      return 1;
    case OrderEntryState::DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

// listen-key

void OrderEntry::get_listen_key() {
  profile_.listen_key([&]() {
    auto method = core::http::Method::POST;
    auto path = "/api/v3/userDataStream"sv;
    auto headers = fmt::format("X-MBX-APIKEY: {}\r\n"sv, security_.get_api_key());
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    connection_(
        "listen_key"sv,
        request,
        [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          server::Trace event(trace_info, response);
          get_listen_key_ack(event, sequence);
        });
  });
}

void OrderEntry::get_listen_key_ack(
    const server::Trace<core::web::Response> &event, [[maybe_unused]] uint32_t sequence) {
  profile_.listen_key_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = OrderEntryState::LISTEN_KEY;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      response.expect(core::http::Status::OK);
      auto listen_key = core::json::Parser::create<json::ListenKey>(response.body());
      server::Trace event(trace_info, listen_key);
      (*this)(event);
      download_.check_relaxed(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void OrderEntry::operator()(const server::Trace<json::ListenKey> &event) {
  auto &[trace_info, listen_key] = event;
  log::info<2>("listen_key={}"sv, listen_key);
  bool initial = listen_key_.empty();
  if (utils::update(listen_key_, listen_key.listen_key)) {
    if (initial) {
      log::info(R"(Listen key has been acquired (value="{}"))"sv, listen_key_);
      ListenKeyUpdate listen_key_update{
          .account = security_.get_account(),
          .listen_key = listen_key.listen_key,
      };
      create_trace_and_dispatch(handler_, trace_info, listen_key_update);
    } else {
      if (ROQ_UNLIKELY(!initial))
        log::info("Listen key has been refreshed!"sv);
    }
  }
  auto now = core::get_system_clock();
  listen_key_refresh_ = now + Flags::rest_listen_key_refresh();
}

void OrderEntry::refresh_listen_key() {
  if (!ready())
    return;
  auto now = core::get_system_clock();
  if (listen_key_refresh_ == listen_key_refresh_.zero() || now < listen_key_refresh_)
    return;
  log::info("Refreshing listen key..."sv);
  listen_key_refresh_ = now + Flags::rest_listen_key_refresh();
  get_listen_key();
  /*
  get<json::ListenKey>([this](auto &promise) {
    try {
      (*this)(promise.get());
    } catch (core::NetworkError &) {
      log::warn("Rescheduling listen key refresh!"sv);
      auto now = core::get_system_clock();
      listen_key_refresh_ = now + Flags::rest_listen_key_refresh();
    }
  });
  */
}

// account

void OrderEntry::get_account() {
  profile_.account([&]() {
    auto method = core::http::Method::GET;
    auto path = "/api/v3/account"sv;
    auto now = core::get_realtime_clock();
    auto [timestamp, signature] = security_.create_signature(now);
    auto query = fmt::format("?{}&signature={}"sv, timestamp, signature);
    auto headers = fmt::format("X-MBX-APIKEY: {}\r\n"sv, security_.get_api_key());
    core::web::Request request{
        .method = method,
        .path = path,
        .query = query,
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    connection_(
        "account"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          server::Trace event(trace_info, response);
          get_account_ack(event, sequence);
        });
  });
}

void OrderEntry::get_account_ack(
    const server::Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.account_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = OrderEntryState::ACCOUNT;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto account = core::json::Parser::create<json::Account>(body, buffer);
      server::Trace event(trace_info, account);
      (*this)(event);
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void OrderEntry::operator()(const server::Trace<json::Account> &event) {
  auto &[trace_info, account] = event;
  log::info<2>("account={}"sv, account);
  for (auto &item : account.balances) {
    FundsUpdate funds_update{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .currency = item.asset,
        .balance = item.free,
        .hold = item.locked,
        .external_account = {},
    };
    create_trace_and_dispatch(handler_, trace_info, funds_update, true);
  }
}

// new-order

void OrderEntry::new_order(
    const Event<CreateOrder> &event, const oms::Order &, const std::string_view &request_id) {
  profile_.new_order([&]() {
    auto &[trace_info, create_order] = event;
    if (!ready())
      throw oms::NotReadyException();
    auto method = core::http::Method::POST;
    auto path = "/api/v3/order"sv;
    auto timestamp = core::get_realtime_clock();
    auto side = json::map(create_order.side).as_raw_text();
    auto type = json::map(create_order.order_type).as_raw_text();
    auto time_in_force = json::map(create_order.time_in_force).as_raw_text();
    auto body = fmt::format(
        R"({{)"
        R"("symbol":"{}",)"
        R"("side":"{}",)"
        R"("type":"{}",)"
        R"("timeInForce":"{}",)"
        R"("quantity":{},)"
        R"("quoteOrderQty":{},)"  // XXX ???
        R"("price":{},)"
        R"("newClientOrderId":"{}")"
        R"("stopPrice":{},)"   // XXX ???
        R"("icebergQty":{},)"  // XXX ???
        R"("recvWindow":{},)"
        R"("timestamp":{})"
        R"(}})"sv,
        create_order.symbol,
        side,
        type,
        time_in_force,
        create_order.quantity,
        0.0,
        create_order.price,
        request_id,
        0.0,
        0.0,
        std::chrono::duration_cast<std::chrono::milliseconds>(Flags::rest_order_recv_window())
            .count(),
        timestamp.count());
    log::debug(R"(body="{}")"sv, body);
    auto headers = fmt::format("X-MBX-APIKEY: {}\r\n"sv, security_.get_api_key());
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
    };
    connection_(request_id, request, [this]([[maybe_unused]] auto &request_id, auto &response) {
      auto trace_info = server::create_trace_info();
      server::Trace event(trace_info, response);
      new_order_ack(event);
    });
  });
}

void OrderEntry::new_order_ack(const server::Trace<core::web::Response> &event) {
  profile_.new_order_ack([&]() {
    auto &[trace_info, response] = event;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto new_order = core::json::Parser::create<json::NewOrder>(body, buffer);
      server::Trace event(trace_info, new_order);
      (*this)(event);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      // XXX HANS ???
    }
  });
}

void OrderEntry::operator()(const server::Trace<json::NewOrder> &event) {
  auto &[trace_info, new_order] = event;
  log::info<1>("new_order={}"sv, new_order);
  throw NotImplementedException();
}

// cancel-order

void OrderEntry::cancel_order(
    const Event<CancelOrder> &,
    const oms::Order &order,
    const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  profile_.cancel_order([&]() {
    auto method = core::http::Method::DELETE;
    auto path = "/api/v3/order"sv;
    if (!ready())
      throw oms::NotReadyException();
    auto timestamp = core::get_realtime_clock();
    auto body = fmt::format(
        R"({{)"
        R"("symbol":"{}",)"
        R"("origClientOrderId":"{}")"
        R"("newClientOrderId":"{}")"
        R"("recvWindow":{},)"
        R"("timestamp":{})"
        R"(}})"sv,
        order.symbol,
        order.external_order_id,
        request_id,
        std::chrono::duration_cast<std::chrono::milliseconds>(Flags::rest_order_recv_window())
            .count(),
        timestamp.count());
    log::debug(R"(body="{}")"sv, body);
    auto headers = fmt::format("X-MBX-APIKEY: {}\r\n"sv, security_.get_api_key());
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
    };
    connection_(request_id, request, [this]([[maybe_unused]] auto &request_id, auto &response) {
      auto trace_info = server::create_trace_info();
      server::Trace event(trace_info, response);
      cancel_order_ack(event);
    });
  });
}

void OrderEntry::cancel_order_ack(const server::Trace<core::web::Response> &event) {
  profile_.cancel_order_ack([&]() {
    auto &[trace_info, response] = event;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      response.expect(core::http::Status::OK);
      auto cancel_order = core::json::Parser::create<json::CancelOrder>(body);
      server::Trace event(trace_info, cancel_order);
      (*this)(event);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      // XXX HANS ???
    }
  });
}

void OrderEntry::operator()(const server::Trace<json::CancelOrder> &event) {
  auto &[trace_info, cancel_order] = event;
  log::info<1>("cancel_order={}"sv, cancel_order);
  throw NotImplementedException();
}

}  // namespace huobi
}  // namespace roq
