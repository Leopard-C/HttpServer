#include "listener.h"
#include "session.h"
#include "server/http_server.h"
#include <boost/asio/strand.hpp>

namespace ic {
namespace server {

Listener::Listener(HttpServer* svr)
    : svr_(svr), acceptor_(net::make_strand(*(svr_->ioc_)))
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
        svr_->logger()->Error(LOG_CTX, "Acceptor open protocol failed, %s", ec.message().c_str());
        return false;
    }

    acceptor_.set_option(net::socket_base::reuse_address(reuse_address), ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "Reuse address failed, %s", ec.message().c_str());
        return false;
    }

    acceptor_.bind(endpoint, ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "Bind address %s:%hu failed", endpoint.address().to_string().c_str(), endpoint.port());
        return false;
    }

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "Listen failed, max_listen_connections:%d", (int)net::socket_base::max_listen_connections);
        return false;
    }

    svr_->logger()->Info(LOG_CTX, "Listening on %s:%hu ...", endpoint.address().to_string().c_str(), endpoint.port());
    DoAccept();

    return true;
}

void Listener::Stop() {
    beast::error_code ec;
    acceptor_.close(ec);
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "Acceptor stop failed, %s", ec.message().c_str());
    }
}

void Listener::DoAccept() {
    acceptor_.async_accept(
        net::make_strand(*(svr_->ioc_)),
        beast::bind_front_handler(&Listener::OnAccept, shared_from_this())
    );
}

void Listener::OnAccept(beast::error_code ec, tcp::socket socket) {
    if (svr_->should_stop()) {
        return;
    }
    if (!acceptor_.is_open()) {
        svr_->logger()->Error(LOG_CTX, "Acceptor is not opened");
        return;
    }
    if (ec) {
        svr_->logger()->Error(LOG_CTX, "OnAccept error, %s", ec.message().c_str());
    }
    else {
        std::make_shared<Session>(std::move(socket), svr_)->Start();
    }
    DoAccept();
}

} // namesapce server
} // namespace ic
