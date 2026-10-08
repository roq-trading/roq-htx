/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download_2.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/htx/gateway/account.hpp"
#include "roq/htx/gateway/shared.hpp"

#include "roq/htx/protocol/json/accounts_ack.hpp"
#include "roq/htx/protocol/json/balance_ack.hpp"
#include "roq/htx/protocol/json/open_orders_ack.hpp"

#include "roq/htx/protocol/json/cancel_all_orders_ack.hpp"
#include "roq/htx/protocol/json/cancel_order_ack.hpp"
#include "roq/htx/protocol/json/place_order_ack.hpp"

namespace roq {
namespace htx {
namespace gateway {

struct OrderEntry final : public Base<OrderEntry>, public server::OrderActionStream, public web::rest::Client::Handler {
  struct Handler {};

  OrderEntry(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  // protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

  // server::OrderActionStream

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Connected> const &) override;
  void operator()(Trace<web::rest::Disconnected> const &) override;
  void operator()(Trace<web::rest::Latency> const &) override;

  // helpers

  // core::Download

  enum class State {
    UNDEFINED = 0,
    ACCOUNTS,
    BALANCE,
    OPEN_ORDERS,
    DONE,
  };

  int32_t download(Trace<State> const &);

  // accounts

  void accounts();
  void accounts_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::AccountsAck> const &);

  // balance

  void balance();
  void balance_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::BalanceAck> const &);

  // open-orders

  void open_orders();
  void open_orders_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::OpenOrdersAck> const &);

  // place-order

  void place_order(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id);
  void place_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<protocol::json::PlaceOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-order

  void cancel_order(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<protocol::json::CancelOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-all-orders

  void cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void cancel_all_orders_ack(Trace<web::rest::Response> const &, std::string_view const &request_id);
  void operator()(Trace<protocol::json::CancelAllOrdersAck> const &);

  // helpers

  void process_response(Trace<web::rest::Response> const &, auto error_handler, auto success_handler);

 private:
  [[maybe_unused]] Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // connection
  std::unique_ptr<web::rest::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile  //
        accounts,
        accounts_ack,                    //
        balance, balance_ack,            //
        open_orders, open_orders_ack,    //
        place_order, place_order_ack,    //
        cancel_order, cancel_order_ack,  //
        cancel_all_orders, cancel_all_orders_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // account
  Account &account_;
  // shared
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download2<State> download_;
  //
  std::string encode_buffer_;
  int64_t account_id_ = {};
};

}  // namespace gateway
}  // namespace htx
}  // namespace roq
