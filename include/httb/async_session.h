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

namespace httb {

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;
namespace net = boost::asio;
namespace beast = boost::beast;
using namespace std::chrono_literals;

class async_session : public std::enable_shared_from_this<async_session> {
public:
    using errorFunc = std::function<void(boost::system::error_code, std::string)>;
    using successFunc = std::function<void(http::response<http::dynamic_body>, size_t)>;

    async_session(net::io_context &ctx, const httb::request &request) :
        m_sslCtx(net::ssl::context::tlsv12),
        m_resolver(ctx),
        m_rawStream(nullptr),
        m_sslStream(nullptr),
        m_rawRequest(request),
        m_request(m_rawRequest.createBeastRequest()) {

        if (m_rawRequest.isSSL()) {
            m_sslStream = new beast::ssl_stream<beast::tcp_stream>(ctx, m_sslCtx);
        } else {
            m_rawStream = new beast::tcp_stream(ctx);
        }
    }

    ~async_session() {
        delete m_sslStream;
        delete m_rawStream;
    }

    void run(errorFunc onError, successFunc onSuccess) {
        m_errorFunc = std::move(onError);
        m_successFunc = std::move(onSuccess);

        v("run", "Resolve host " + m_rawRequest.getHost());

        m_resolver.async_resolve(m_rawRequest.getHost(), m_rawRequest.getPortString().c_str(),
                                 std::bind(&async_session::onResolve,
                                           shared_from_this(),
                                           std::placeholders::_1,
                                           std::placeholders::_2));

    }

    void setVerbose(bool verbose) {
        m_verbose = verbose;
    }

    void setConnectionTimeout(std::chrono::seconds timeout) {
        m_connectionTimeout = std::move(timeout);
    }

    void setReadTimeout(std::chrono::seconds timeout) {
        m_readTimeout = std::move(timeout);
    }
private:
    boost::asio::ssl::context m_sslCtx;
    tcp::resolver m_resolver;
    beast::tcp_stream *m_rawStream;
    beast::ssl_stream<beast::tcp_stream> *m_sslStream;
    beast::flat_buffer m_buffer; // (Must persist between reads)
    httb::request m_rawRequest;
    const http::request<http::string_body> m_request;
    http::response<http::dynamic_body> m_response;
    errorFunc m_errorFunc;
    successFunc m_successFunc;
    bool m_verbose = false;
    std::chrono::seconds m_connectionTimeout = 30s;
    std::chrono::seconds m_readTimeout = 30s;
    const std::vector<boost::system::error_code> m_ignoredErrors{
        boost::asio::ssl::error::stream_truncated,
        boost::asio::error::eof,
    };

    bool isIgnoreError(boost::system::error_code ec) {
        return std::find(m_ignoredErrors.begin(), m_ignoredErrors.end(), ec) != m_ignoredErrors.end();
    }

    void fail(boost::system::error_code ec, char const *where) {
        if (m_errorFunc) {
            m_errorFunc(ec, std::string(where));
        }
    }

    void v(const std::string &tag, const std::string &msg) {
        if (!m_verbose) {
            return;
        }
        std::cout << tag << ": " << msg << std::endl;
    }

    void onRead(boost::system::error_code ec, std::size_t bytesTransferred) {
        if (ec && !isIgnoreError(ec)) {
            fail(ec, "read");
            return;
        }

        v("onRead", "Shutting down");

        // Don't shutdown ssl stream - it's bad idea, you will get inifinite waiting for server closing ssl. Close socket directly with no worries
        getStream()->socket().shutdown(tcp::socket::shutdown_both, ec);

        if (ec && !isIgnoreError(ec) && ec != boost::system::errc::not_connected) {
            fail(ec, "shutdown");
            return;
        }

        if (m_successFunc) {
            m_successFunc(m_response, bytesTransferred);
        }

        // here everything gracefully closed
    }

    void onWrite(boost::system::error_code ec, std::size_t) {
        if (ec && !isIgnoreError(ec)) {
            fail(ec, "write");
            return;
        }

        v("onWritten", "Read response...");
        getStream()->expires_after(m_readTimeout);
        if (m_rawRequest.isSSL()) {
            http::async_read(*m_sslStream, m_buffer, m_response,
                             std::bind(&async_session::onRead,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
        } else {
            http::async_read(*m_rawStream, m_buffer, m_response,
                             std::bind(&async_session::onRead,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
        }
    }

    void onConnect(boost::system::error_code ec) {
        if (ec && !isIgnoreError(ec)) {
            fail(ec, "connect");
            return;
        }

        getStream()->expires_after(m_connectionTimeout);

        if(m_verbose) {
            std::stringstream verboseStream;
            verboseStream << m_request;
            std::string verboseString = verboseStream.str();

            if (verboseStream.str().size() >= 1024) {
                verboseString = verboseString.substr(0, 1023);
                verboseString += " ...(too big for output)";
            }
            std::cout << verboseStream.str();
        }

        if (m_rawRequest.isSSL()) {
            v("onConnected", "Handshaking...");
            m_sslStream->async_handshake(ssl::stream_base::client,
                                         std::bind(&async_session::onHandshake,
                                                   shared_from_this(),
                                                   std::placeholders::_1)
            );
            return;
        }
        http::async_write(*getStream(), m_request,
                          std::bind(&async_session::onWrite,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
    }

    void onHandshake(boost::system::error_code ec) {
        if (ec && !isIgnoreError(ec)) {
            fail(ec, "handshake");
            return;
        }

        http::async_write(*m_sslStream, m_request,
                          std::bind(&async_session::onWrite,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
    }

    void onResolve(boost::system::error_code ec, tcp::resolver::results_type results) {
        if (ec && !isIgnoreError(ec)) {
            fail(ec, "resolve");
            return;
        }

        getStream()->expires_after(std::chrono::seconds(10));

        v("onResolved", "Connecting to host...");
        getStream()->async_connect(results.begin(), results.end(),
                                   std::bind(&async_session::onConnect, shared_from_this(), std::placeholders::_1));

    }

    beast::tcp_stream *getStream() {
        return m_rawRequest.isSSL() ? &m_sslStream->next_layer() : m_rawStream;
    }
};

} // httb

#endif //HTTB_ASYNC_SESSION_H
