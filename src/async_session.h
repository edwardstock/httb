/*!
 * httb.
 * session.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_ASYNC_SESSION_H
#define HTTB_ASYNC_SESSION_H

#include "httb/request.h"
#include "httb/types.h"

#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/string.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <istream>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

namespace httb {

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;
namespace net = boost::asio;
namespace beast = boost::beast;
using namespace std::chrono_literals;

class async_session : public std::enable_shared_from_this<async_session> {
public:
    /// \brief single constructor
    /// \param ctx boost asio io_context
    /// \param request
    async_session(net::io_context& ctx, httb::request request, std::chrono::seconds conn_tout, std::chrono::seconds read_tout);
    virtual ~async_session();

    /// \brief Start executing http(s) request
    /// \param onError error callback
    /// \param onSuccess success callback
    void run(error_func_t onError, success_func_t onSuccess);

    /// \brief Enable verbose output
    /// \param verbose
    void set_verbose(bool verbose);

    /// \brief Set progress callback
    /// \param progress
    void set_on_progress_cb(progress_func_t progress);

private:
    boost::asio::io_service::strand m_strand;
    boost::asio::ssl::context m_ssl_ctx;
    tcp::resolver m_resolver;
    beast::tcp_stream* m_stream_raw;
    beast::ssl_stream<beast::tcp_stream>* m_stream_ssl;
    beast::multi_buffer m_buffer; // (Must persist between reads)
    const httb::request m_request_raw;
    const http::request<request_body_type> m_request;
    http::response_parser<httb::response_body_type> m_response;
    http::response_parser<httb::response_file_body_type> m_file_response;
    error_func_t m_error_func;
    success_func_t m_success_func;
    progress_func_t m_progress_func;
    bool m_verbose = false;
    std::chrono::seconds m_conn_timeout = 30s;
    std::chrono::seconds m_read_timeout = 30s;
    const std::vector<boost::system::error_code> m_ignored_errors{
        boost::asio::ssl::error::stream_truncated,
        boost::asio::error::eof,
    };

    inline bool is_ignored_error(boost::system::error_code ec);
    inline void fail(boost::system::error_code ec, char const* where);

    void v(const std::string& tag, const std::string& msg);

    void read_response();
    void read_response_ssl();
    void read_response_plain();

    void on_resolve(boost::system::error_code ec, tcp::resolver::results_type results);
    void on_connect(boost::system::error_code ec);
    void on_ssl_handshake(boost::system::error_code ec);
    void on_write(boost::system::error_code ec, std::size_t);
    void on_read(boost::system::error_code ec, std::size_t bytesTransferred);

    beast::tcp_stream* stream();
};

} // namespace httb

#endif //HTTB_ASYNC_SESSION_H
