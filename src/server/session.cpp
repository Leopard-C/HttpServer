#include "session.h"
#include "server/http_server.h"
#include "server/request_raw.h"
#include "server/router.h"
#include <chrono>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <log/logger.h>

namespace ic {
namespace server {

Session::Session(tcp::socket&& socket, HttpServer* svr)
    : svr_(svr), stream_(std::move(socket))
{
    LDebug("New session");
}

Session::~Session() {
    LDebug("Destroy session");
}

void Session::Run() {
    net::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(&Session::DoRead, shared_from_this())
    );
}

void Session::DoRead() {
    LTrace("DoRead ({})", stream_.socket().native_handle());
    parser_ = std::make_shared<http::request_parser<http::string_body>>();
    parser_->eager(true);
    /* 限制body大小 */
    uint64_t body_limit = svr_->config().body_limit();
    parser_->body_limit(body_limit > 0 ? body_limit : 1024 * 1024 * 10);   /* 默认限制大小10MB */
    /* TCP连接超时(keep-alive最长时间) */
    unsigned int tcp_stream_timeout_ms = svr_->config().tcp_stream_timeout_ms();
    if (tcp_stream_timeout_ms > 0) {
        stream_.expires_after(std::chrono::milliseconds(tcp_stream_timeout_ms));
    }
    else {
        stream_.expires_never();
    }
    http::async_read(stream_, buffer_, *parser_, beast::bind_front_handler(&Session::OnRead, shared_from_this()));
}

void Session::OnRead(beast::error_code ec, size_t bytes_transferred) {
    LTrace("OnRead ({})", stream_.socket().native_handle());
    res_ = std::make_shared<Response>(svr_);

    boost::ignore_unused(bytes_transferred);
    if (ec) {
        return ProcessReadError(ec);
    }

    auto req_raw = (RequestRaw*)(&(parser_->get()));
    res_->set_keep_alive(req_raw->keep_alive());

    auto client_ip = stream_.socket().remote_endpoint().address();
    req_ = std::make_shared<Request>(svr_, req_raw, client_ip.to_string());

    svr_->StartRequest(*req_);
    if (PreHandleRequest()) {
        HandleRequest();
    }
    svr_->FinishRequest(*req_);

    SendResponse();
}

void Session::ProcessReadError(beast::error_code ec) {
    LTrace("OnRead error, {}", ec.message());
    if (ec == beast::error::timeout) {
        return;
    }
    else if (ec == http::error::end_of_stream) {
        return DoClose();
    }
    else if (ec == http::error::body_limit) {
        LError("Body limit");
        res_->SetStringBody(413, "body limit exceeded", "text/plain");
        close_ = true;  /* 返回413后关闭连接，不然会出现 http:error::bad_method 错误 */
        return SendResponse();
    }
    else {
        LError("OnRead error, {}", ec.message());
        return;
    }
}

void Session::OnWrite(bool close, beast::error_code ec, size_t bytes_transferred) {
    LTrace("OnWrite ({})", stream_.socket().native_handle());
    boost::ignore_unused(bytes_transferred);
    if (string_res_) {
        string_res_.reset();
    }
    if (file_res_) {
        if (file_res_->body().is_open()) {
            file_res_->body().close();
        }
        file_res_.reset();
    }
    if (ec) {
        LError("OnWrite error, {}", ec.message());
        return;
    }
    if (close || close_) {
        return DoClose();
    }
    DoRead();
}

void Session::DoClose() {
    LTrace("DoClose ({})", stream_.socket().native_handle());
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_both, ec);
    if (ec) {
        LError("Socket.ShutdownBoth failed, {}", ec.message());
    }
}

/**
 * @brief 预处理请求.
 */
bool Session::PreHandleRequest() {
    /* 检查是否命中路由 */
    if (!svr_->router()->HitRoute(*req_, *res_)) {
        return false;
    }

    /* 请求拦截器(1) */
    if (svr_->cb_before_parse_body_ && !svr_->cb_before_parse_body_(*req_, *res_)) {
        return false;
    }

    /* 解析body */
    if (!req_->ParseBody()) {
        req_->LogAccess();
        res_->SetBadRequest("Bad request!");
        LError("Bad request. Parse request body failed");
        return false;
    }

    /* 打印日志 */
    req_->LogAccess();

    /* 请求拦截器(2) */
    if (svr_->cb_before_handle_request_ && !svr_->cb_before_handle_request_(*req_, *res_)) {
        return false;
    }

    return true;
}

/**
 * @brief 处理请求.
 */
void Session::HandleRequest() {
    auto start = std::chrono::system_clock::now();
    req_->route()->Invoke(*req_, *res_);
    auto finish = std::chrono::system_clock::now();
    req_->time_consumed_handle_ = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
}

/**
 * @brief 返回响应内容.
 */
void Session::SendResponse() {
    /* 响应拦截器 */
    svr_->cb_before_send_response_ && svr_->cb_before_send_response_(*req_, *res_);
    return res_->is_file_body_ ? SendFileBodyResponse() : SendStringBodyResponse();
}

/**
 * @brief 返回文件内容.
 */
void Session::SendFileBodyResponse() {
    http::file_body::value_type file;
    beast::error_code ec;
    file.open(res_->filename_.c_str(), beast::file_mode::read, ec);
    if (ec) {
        res_->SetStringBody(404U); // return 404 Not Found
        return SendStringBodyResponse();
    }
    file_res_ = std::make_shared<http::response<http::file_body>>();
    file_res_->keep_alive(res_->keep_alive_);
    file_res_->result(res_->status_code_);
    file_res_->body() = std::move(file);
    for (const auto& p : res_->headers_) {
        file_res_->set(p.first, p.second);
    }
    file_res_->prepare_payload();
    file_serializer_ = std::make_shared<http::response_serializer<http::file_body>>(*file_res_);
    http::async_write(
        stream_,
        *file_serializer_,
        beast::bind_front_handler(&Session::OnWrite, shared_from_this(), file_res_->need_eof())
    );
}

/**
 * @brief 返回文本内容.
 */
void Session::SendStringBodyResponse() {
    string_res_ = std::make_shared<http::response<http::string_body>>();
    string_res_->keep_alive(res_->keep_alive_);
    string_res_->result(res_->status_code_);
    string_res_->body().swap(res_->string_body_);
    string_res_->prepare_payload();
    for (const auto& p : res_->headers_) {
        string_res_->set(p.first, p.second);
    }
    http::async_write(
        stream_,
        *string_res_,
        beast::bind_front_handler(&Session::OnWrite, shared_from_this(), string_res_->need_eof())
    );
}

} // namespace server
} // namespace ic
