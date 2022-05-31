/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/huobi/drop_copy.hpp"

#include "roq/mask.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/huobi/flags.hpp"

#include "roq/huobi/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace huobi {

namespace {
auto const NAME = "ex"sv;
const Mask SUPPORTS{
    SupportType::FUNDS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(std::string_view const &group, std::string_view const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context, auto const &listen_key) {
  assert(!std::empty(listen_key));
  auto uri = Flags::ws_order_uri();
  auto query = fmt::format("?streams={}"sv, listen_key);
  core::web::ClientSocket::Config config{
      .always_reconnect = true,
      .connection_timeout = server::Flags::net_connection_timeout(),
      .disconnect_on_idle_timeout = {},
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .uris = {&uri, 1},
      .query = query,
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return core::web::ClientSocket{handler, context, config, []() { return std::string(); }};
}
}  // namespace

DropCopy::DropCopy(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared,
    std::string_view const &listen_key)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      connection_(create_connection(*this, context, listen_key)), decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .ping = create_metrics(name_, "ping"sv),
          .error = create_metrics(name_, "error"sv),
          .outbound_account_info = create_metrics(name_, "outbound_account_info"sv),
          .outbound_account_position = create_metrics(name_, "outbound_account_position"sv),
          .balance_update = create_metrics(name_, "balance_update"sv),
          .execution_report = create_metrics(name_, "execution_report"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
      },
      security_(security), shared_(shared), download_({}, [this](auto state) { return download(state); }) {
}

bool DropCopy::ready() const {
  return connection_.ready();
}

void DropCopy::operator()(Event<Start> const &) {
  connection_.start();
}

void DropCopy::operator()(Event<Stop> const &) {
  connection_.stop();
}

void DropCopy::operator()(Event<Timer> const &event) {
  connection_.refresh(event.value.now);
}

void DropCopy::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.outbound_account_info, metrics::PROFILE)
      .write(profile_.outbound_account_position, metrics::PROFILE)
      .write(profile_.balance_update, metrics::PROFILE)
      .write(profile_.execution_report, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void DropCopy::operator()(core::web::ClientSocket::Connected const &) {
}

void DropCopy::operator()(core::web::ClientSocket::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
}

void DropCopy::operator()(core::web::ClientSocket::Ready const &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void DropCopy::operator()(core::web::ClientSocket::Close const &) {
}

void DropCopy::operator()(core::web::ClientSocket::Latency const &latency) {
  auto trace_info = server::create_trace_info();
  const ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = security_.get_account(),
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(core::web::ClientSocket::Text const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(core::web::ClientSocket::Binary const &) {
  log::fatal("Not implemented"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    const StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t DropCopy::download(DropCopyState state) {
  switch (state) {
    using enum DropCopyState;
    case UNDEFINED:
      assert(false);
      break;
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      // subscribe(symbols_);
      return {};
  }
  assert(false);
  return {};
}

void DropCopy::parse(std::string_view const &message) {
  profile_.parse([&]() {
    try {
      auto trace_info = server::create_trace_info();
      core::json::Buffer buffer(decode_buffer_);
      json::Parser::dispatch(*this, message, buffer, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void DropCopy::operator()(Trace<json::Ping const> const &event) {
  profile_.ping([&]() {
    auto &[trace_info, ping] = event;
    log::debug("ping={}"sv, ping);
    // send_pong(ping.timestamp);
  });
}

void DropCopy::operator()(Trace<json::Error const> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
  });
}

void DropCopy::operator()(Trace<json::Subbed const> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::BBO const> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Trade const> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Detail const> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Ticker const> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::MBP const> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::MBPSnapshot const> const &) {
  log::fatal("Unexpected"sv);
}

}  // namespace huobi
}  // namespace roq
