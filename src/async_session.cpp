/*!
 * httb.
 * async_session.cpp
 *
 * \date 11/06/2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "async_session.h"

#include <utility>

httb::async_session::async_session(boost::asio::io_context& ctx, httb::request request, std::chrono::seconds conn_tout, std::chrono::seconds read_tout)
    : m_strand(ctx),
      m_ssl_ctx(net::ssl::context::tlsv12),
      m_resolver(m_strand),
      m_stream_raw(nullptr),
      m_stream_ssl(nullptr),
      m_request_raw(std::move(request)),
      m_request(m_request_raw.to_beast_request()),
      m_conn_timeout(conn_tout),
      m_read_timeout(read_tout) {

    m_response.body_limit(std::numeric_limits<std::uint64_t>::max());

    if (m_request_raw.is_ssl()) {
        m_stream_ssl = new beast::ssl_stream<beast::tcp_stream>(m_strand, m_ssl_ctx);
    } else {
        m_stream_raw = new beast::tcp_stream(m_strand);
    }
}
httb::async_session::~async_session() {
    delete m_stream_ssl;
    delete m_stream_raw;
}

void httb::async_session::read_response() {
    if (m_request_raw.is_ssl()) {
        read_response_ssl();
    } else {
        read_response_plain();
    }
}

void httb::async_session::read_response_ssl() {
    if (m_progress_func) {
        http::async_read_some(*m_stream_ssl, m_buffer, m_response,
                              std::bind(&async_session::on_read,
                                        shared_from_this(),
                                        std::placeholders::_1,
                                        std::placeholders::_2));
    } else {
        // read full content is no progress callback set
        http::async_read(*m_stream_ssl, m_buffer, m_response,
                         std::bind(&async_session::on_read,
                                   shared_from_this(),
                                   std::placeholders::_1,
                                   std::placeholders::_2));
    }
}

void httb::async_session::read_response_plain() {
    if (m_progress_func) {
        http::async_read_some(*m_stream_raw, m_buffer, m_response,
                              std::bind(&async_session::on_read,
                                        shared_from_this(),
                                        std::placeholders::_1,
                                        std::placeholders::_2));
    } else {
        http::async_read(*m_stream_raw, m_buffer, m_response,
                         std::bind(&async_session::on_read,
                                   shared_from_this(),
                                   std::placeholders::_1,
                                   std::placeholders::_2));
    }
}

void httb::async_session::run(httb::error_func_t onError, httb::success_func_t onSuccess) {
    m_error_func = std::move(onError);
    m_success_func = std::move(onSuccess);

    v("run", "Resolve host " + m_request_raw.get_host());

    m_resolver.async_resolve(m_request_raw.get_host(), m_request_raw.get_port_str().c_str(),
                             std::bind(&async_session::on_resolve,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
}

void httb::async_session::on_resolve(boost::system::error_code ec,
                                     net::ip::basic_resolver<net::ip::tcp, net::executor>::results_type results) {
    if (ec && !is_ignored_error(ec)) {
        fail(ec, "resolve");
        return;
    }

    stream()->expires_after(std::chrono::seconds(m_conn_timeout));

    v("on_resolved", "Connecting to host...");
    stream()->async_connect(results.begin(), results.end(),
                            std::bind(&async_session::on_connect, shared_from_this(), std::placeholders::_1));
}

void httb::async_session::on_connect(boost::system::error_code ec) {
    if (ec && !is_ignored_error(ec)) {
        fail(ec, "connect");
        return;
    }

    if (m_verbose) {
        std::stringstream verboseStream;
        verboseStream << m_request;
        std::string verboseString = verboseStream.str();

        if (verboseStream.str().size() >= 1024) {
            verboseString = verboseString.substr(0, 1023);
            verboseString += " ...(too big for output)";
        }
        std::cout << verboseStream.str();
    }

    if (m_request_raw.is_ssl()) {
        v("on_connected", "Handshaking...");
        m_stream_ssl->async_handshake(ssl::stream_base::client,
                                      std::bind(&async_session::on_ssl_handshake,
                                                shared_from_this(),
                                                std::placeholders::_1));
        return;
    }

    stream()->expires_never();
    http::async_write(*stream(), m_request,
                      std::bind(&async_session::on_write,
                                shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2));
}

void httb::async_session::on_ssl_handshake(boost::system::error_code ec) {
    if (ec && !is_ignored_error(ec)) {
        fail(ec, "handshake");
        return;
    }

    stream()->expires_never();
    http::async_write(*m_stream_ssl, m_request,
                      std::bind(&async_session::on_write,
                                shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2));
}

void httb::async_session::on_write(boost::system::error_code ec, std::size_t) {
    if (ec && !is_ignored_error(ec)) {
        fail(ec, "write");
        return;
    }

    v("on_write", "Read response...");
    stream()->expires_after(m_read_timeout);
    read_response();
}

void httb::async_session::on_read(boost::system::error_code ec, std::size_t bytesTransferred) {
    if (ec && !is_ignored_error(ec)) {
        fail(ec, "read");
        return;
    }

    if (m_response.is_done()) {
        v("on_read", "Shutting down");

        // Don't shutdown ssl stream - it's bad idea, you will get inifinite waiting for server closing ssl. Close socket directly with no worries
        stream()->socket().shutdown(tcp::socket::shutdown_both, ec);

        if (ec && !is_ignored_error(ec) && ec != boost::system::errc::not_connected) {
            fail(ec, "shutdown");
            return;
        }

        if (m_progress_func) {
            uint64_t total_len = m_response.content_length().value_or(0ULL);
            if (total_len == 0) {
                m_progress_func(0ULL, 0ULL, 0.0);
            } else {
                m_progress_func(total_len, total_len, 1.0);
            }
        }

        if (m_success_func) {
            m_success_func(m_response.release(), bytesTransferred);
        }
        // here everything gracefully closed
    } else {
        if (m_progress_func) {
            uint64_t total_len = m_response.content_length().value_or(0ULL);
            uint64_t remain = m_response.content_length_remaining().value_or(0ULL);
            if (total_len == 0) {
                m_progress_func(0ULL, 0ULL, 0.0);
            } else {
                const uint64_t loaded = total_len - remain;
                const double progress = (double) loaded / (double) total_len;
                m_progress_func(loaded, total_len, progress);
            }
        }

        read_response();
    }
}

void httb::async_session::set_verbose(bool verbose) {
    m_verbose = verbose;
}

void httb::async_session::set_on_progress_cb(httb::progress_func_t progress) {
    m_progress_func = progress;
}

bool httb::async_session::is_ignored_error(boost::system::error_code ec) {
    return std::find(m_ignored_errors.begin(), m_ignored_errors.end(), ec) != m_ignored_errors.end();
}

void httb::async_session::fail(boost::system::error_code ec, char const* where) {
    if (m_error_func) {
        m_error_func(ec, std::string(where));
    }
}
void httb::async_session::v(const std::string& tag, const std::string& msg) {
    if (!m_verbose) {
        return;
    }
    std::cout << tag << ": " << msg << std::endl;
}

boost::beast::tcp_stream* httb::async_session::stream() {
    return m_request_raw.is_ssl() ? &m_stream_ssl->next_layer() : m_stream_raw;
}
