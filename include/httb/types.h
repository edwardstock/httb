/*!
 * httb.
 * types.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_TYPES_H
#define HTTB_TYPES_H

#include <boost/beast.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/optional.hpp>

namespace httb {

#ifndef HTTB_USE_OWN_BODY_TYPES
using request_body_type = boost::beast::http::string_body;
using response_body_type = boost::beast::http::dynamic_body;
using response_file_body_type = boost::beast::http::file_body;
using response_t = boost::beast::http::response<httb::response_body_type>;
#endif

using context = boost::asio::io_context;

}

#endif //HTTB_TYPES_H
