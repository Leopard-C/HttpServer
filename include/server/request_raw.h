/**
 * @file request_raw.h
 * @brief boost.beast库提供的request对象.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_REQUEST_RAW_H_
#define IC_SERVER_REQUEST_RAW_H_
#include <boost/asio/ip/address.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace ic {
namespace server {

/**
 * @brief boost.beast库提供的request对象.
 */
class RequestRaw : public boost::beast::http::request<boost::beast::http::string_body> { };

} // namespace server
} // namespace ic

#endif // IC_SERVER_REQUEST_RAW_H_
