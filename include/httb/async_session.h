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

#include <memory>
#include <utility>
#include <string>
#include <iostream>
#include <istream>
#include <ostream>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/core/string.hpp>
#include <thread>
#include <future>
#include <toolboxpp.hpp>
#include <type_traits>
#include <functional>

#include "helpers.hpp"
#include "httb/request.h"
#include "httb/timer.h"
#include "httb/types.h"

namespace httb {

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;
namespace net = boost::asio;
namespace beast = boost::beast;
using namespace std::chrono_literals;

class async_session : public std::enable_shared_from_this<async_session> {
public:
    /// \brief Error callback
    using error_func_t = std::function<void(boost::system::error_code, std::string)>;
    /// \brief Success callback
    using success_func_t = std::function<void(httb::response_t &&, size_t)>;
    /// \brief Progress callback
    using progress_func_t = std::function<void(uint64_t, uint64_t, double)>;

    /// \brief single constructor
    /// \param ctx boost asio io_context
    /// \param request
    async_session(net::io_context &ctx, const httb::request &request);
    virtual ~async_session();

    /// \brief Start executing http(s) request
    /// \param onError error callback
    /// \param onSuccess success callback
    void run(error_func_t onError, success_func_t onSuccess);

    /// \brief Enable verbose output
    /// \param verbose
    void setVerbose(bool verbose);

    /// \brief Set progress callback
    /// \param progress
    void setOnProgress(progress_func_t progress);

    /// \brief Set connect timeout (not read timeout)
    /// \param timeout
    void setConnectionTimeout(std::chrono::seconds timeout);

    /// \brief Set response read timeout
    /// \param timeout
    void setReadTimeout(std::chrono::seconds timeout);
private:
    boost::asio::io_service::strand m_strand;
    boost::asio::ssl::context m_sslCtx;
    tcp::resolver m_resolver;
    beast::tcp_stream *m_streamRaw;
    beast::ssl_stream<beast::tcp_stream> *m_streamSSL;
    beast::multi_buffer m_buffer; // (Must persist between reads)
    const httb::request m_requestRaw;
    const http::request<request_body_type> m_request;
    http::response_parser<httb::response_body_type> m_response;
    http::response_parser<httb::response_file_body_type> m_file_response;
    error_func_t m_errorFunc;
    success_func_t m_successFunc;
    progress_func_t m_progressFunc;
    bool m_verbose = false;
    std::chrono::seconds m_connectionTimeout = 30s;
    std::chrono::seconds m_readTimeout = 30s;
    const std::vector<boost::system::error_code> m_ignoredErrors{
        boost::asio::ssl::error::stream_truncated,
        boost::asio::error::eof,
    };

    inline bool isIgnoredError(boost::system::error_code ec);
    inline void fail(boost::system::error_code ec, char const *where);

    void v(const std::string &tag, const std::string &msg);

    void readResponse();
    void readSSLResponse();
    void readPlainResponse();

    void onResolve(boost::system::error_code ec, tcp::resolver::results_type results);
    void onConnect(boost::system::error_code ec);
    void onHandshake(boost::system::error_code ec);
    void onWrite(boost::system::error_code ec, std::size_t);
    void onRead(boost::system::error_code ec, std::size_t bytesTransferred);

    beast::tcp_stream *getStream();
};

} // httb

#endif //HTTB_ASYNC_SESSION_H
