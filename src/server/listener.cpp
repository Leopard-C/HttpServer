#include "listener.h"
#include "http_server.h"
#include "session.h"
#include <boost/asio/strand.hpp>
#include <log/logger.h>

namespace ic {
namespace server {

Listener::Listener(HttpServer* svr, tcp::endpoint endpoint)
    : svr_(svr), endpoint_(endpoint), acceptor_(net::make_strand(*(svr->ioc_)))
{
}

bool Listener::Run() {
    beast::error_code ec;

    acceptor_.open(endpoint_.protocol(), ec);
    if (ec) {
        LError("acceptor open protocol failed, {}", ec.message());
        return false;
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        LError("reuse address failed, {}", ec.message());
        return false;
    }

    acceptor_.bind(endpoint_, ec);
    if (ec) {
        LError("Bind address {}:{} failed", endpoint_.address().to_string(), endpoint_.port());
        return false;
    }

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        LError("Listen failed, max_listen_connections:{}", int(net::socket_base::max_listen_connections));
        return false;
    }

    LInfo("Listening on {}:{} ...", endpoint_.address().to_string(), endpoint_.port());
    DoAccept();
    return true;
}

void Listener::DoAccept() {
    acceptor_.async_accept(
        net::make_strand(*(svr_->ioc_)),
        beast::bind_front_handler(&Listener::OnAccept, shared_from_this())
    );
}

void Listener::OnAccept(beast::error_code ec, tcp::socket socket) {
    if (!acceptor_.is_open()) {
        LCritical("Acceptor is not opened");
        return;
    }
    if (ec) {
        LCritical("OnAccept error, {}", ec.message());
        return;
    }
    std::make_shared<Session>(std::move(socket), svr_)->Run();
    DoAccept();
}

} // namesapce server
} // namespace ic
