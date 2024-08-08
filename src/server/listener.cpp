#include "listener.h"
#include "session.h"
#include "server/http_server.h"
#include <boost/asio/strand.hpp>

namespace ic {
namespace server {

Listener::Listener(HttpServer* svr)
    : svr_(svr), is_running_(false), acceptor_(net::make_strand(*(svr_->ioc_)))
{
}

bool Listener::Run(const std::string& ip, unsigned short port, bool reuse_address) {
    beast::error_code ec;
    net::ip::address address = net::ip::make_address(ip, ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "Invalid address %s", ip.c_str());
        return false;
    }

    tcp::endpoint endpoint(address, port);

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "acceptor open protocol failed, %s", ec.message().c_str());
        return false;
    }

    acceptor_.set_option(net::socket_base::reuse_address(reuse_address), ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "reuse address failed, %s", ec.message().c_str());
        return false;
    }

    acceptor_.bind(endpoint, ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "bind address %s:%hu failed", endpoint.address().to_string().c_str(), endpoint.port());
        return false;
    }

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "listen failed, max_listen_connections:%d", (int)net::socket_base::max_listen_connections);
        return false;
    }

    svr_->logger()->Info(LOG_CTX, "Listening on %s:%hu ...", endpoint.address().to_string().c_str(), endpoint.port());
    DoAccept();

    is_running_ = true;
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
