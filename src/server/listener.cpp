#include "listener.h"
#include "session.h"
#include "server/http_server.h"
#include <boost/asio/strand.hpp>

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
        svr_->logger()->Error(LOG_CTX, "acceptor open protocol failed, %s", ec.message().c_str());
        return false;
    }

    acceptor_.set_option(net::socket_base::reuse_address(svr_->config().reuse_address()), ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "reuse address failed, %s", ec.message().c_str());
        return false;
    }

    acceptor_.bind(endpoint_, ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "bind address %s:%u failed",
            endpoint_.address().to_string().c_str(), (unsigned int)endpoint_.port());
        return false;
    }

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "listen failed, max_listen_connections:%d", (int)net::socket_base::max_listen_connections);
        return false;
    }

    svr_->logger()->Info(LOG_CTX, "Listening on %s:%u ...", endpoint_.address().to_string().c_str(), (unsigned int)endpoint_.port());
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
        svr_->logger()->Error(LOG_CTX, "Acceptor is not opened");
        return;
    }
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "OnAccept error, %s", ec.message().c_str());
        return;
    }
    std::make_shared<Session>(std::move(socket), svr_)->Run();
    DoAccept();
}

} // namesapce server
} // namespace ic
