/*!
 * httb.
 * async_session.cpp
 *
 * \date 11/06/2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "httb/async_session.h"

httb::async_session::async_session(boost::asio::io_context &ctx, const httb::request &request) :
    m_strand(ctx),
    m_sslCtx(net::ssl::context::tlsv12),
    m_resolver(m_strand),
    m_streamRaw(nullptr),
    m_streamSSL(nullptr),
    m_requestRaw(request),
    m_request(m_requestRaw.createBeastRequest()) {
    m_response.body_limit(std::numeric_limits<std::uint64_t>::max());

    if (m_requestRaw.isSSL()) {
        m_streamSSL = new beast::ssl_stream<beast::tcp_stream>(m_strand, m_sslCtx);
    } else {
        m_streamRaw = new beast::tcp_stream(m_strand);
    }
}
httb::async_session::~async_session() {
    delete m_streamSSL;
    delete m_streamRaw;
}

void httb::async_session::readResponse() {
    if (m_requestRaw.isSSL()) {
        readSSLResponse();
    } else {
        readPlainResponse();
    }
}

void httb::async_session::readSSLResponse() {
    if (m_progressFunc) {
        http::async_read_some(*m_streamSSL, m_buffer, m_response,
                              std::bind(&async_session::onRead,
                                        shared_from_this(),
                                        std::placeholders::_1,
                                        std::placeholders::_2)
        );
    } else {
        // read full content is no progress callback set
        http::async_read(*m_streamSSL, m_buffer, m_response,
                         std::bind(&async_session::onRead,
                                   shared_from_this(),
                                   std::placeholders::_1,
                                   std::placeholders::_2));
    }
}

void httb::async_session::readPlainResponse() {
    if(m_progressFunc) {
        http::async_read_some(*m_streamRaw, m_buffer, m_response,
                              std::bind(&async_session::onRead,
                                        shared_from_this(),
                                        std::placeholders::_1,
                                        std::placeholders::_2));
    } else {
        http::async_read(*m_streamRaw, m_buffer, m_response,
                         std::bind(&async_session::onRead,
                                   shared_from_this(),
                                   std::placeholders::_1,
                                   std::placeholders::_2));
    }
}

void httb::async_session::run(httb::async_session::error_func_t onError, httb::async_session::success_func_t onSuccess) {
    m_errorFunc = std::move(onError);
    m_successFunc = std::move(onSuccess);

    v("run", "Resolve host " + m_requestRaw.getHost());

    m_resolver.async_resolve(m_requestRaw.getHost(), m_requestRaw.getPortString().c_str(),
                             std::bind(&async_session::onResolve,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));

}

void httb::async_session::onResolve(boost::system::error_code ec,
                                    boost::asio::ip::basic_resolver<boost::asio::ip::tcp,
                                                                    boost::asio::executor>::results_type results) {
    if (ec && !isIgnoredError(ec)) {
        fail(ec, "resolve");
        return;
    }

    getStream()->expires_after(std::chrono::seconds(10));

    v("onResolved", "Connecting to host...");
    getStream()->async_connect(results.begin(), results.end(),
                               std::bind(&async_session::onConnect, shared_from_this(), std::placeholders::_1));

}

void httb::async_session::onConnect(boost::system::error_code ec) {
    if (ec && !isIgnoredError(ec)) {
        fail(ec, "connect");
        return;
    }

    getStream()->expires_after(m_connectionTimeout);

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

    if (m_requestRaw.isSSL()) {
        v("onConnected", "Handshaking...");
        m_streamSSL->async_handshake(ssl::stream_base::client,
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

void httb::async_session::onHandshake(boost::system::error_code ec) {
    if (ec && !isIgnoredError(ec)) {
        fail(ec, "handshake");
        return;
    }

    http::async_write(*m_streamSSL, m_request,
                      std::bind(&async_session::onWrite,
                                shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2));
}

void httb::async_session::onWrite(boost::system::error_code ec, std::size_t) {
    if (ec && !isIgnoredError(ec)) {
        fail(ec, "write");
        return;
    }

    v("onWritten", "Read response...");
    getStream()->expires_after(m_readTimeout);
    readResponse();
}

void httb::async_session::onRead(boost::system::error_code ec, std::size_t bytesTransferred) {
    if (ec && !isIgnoredError(ec)) {
        fail(ec, "read");
        return;
    }

    if (m_response.is_done()) {
        v("onRead", "Shutting down");

        // Don't shutdown ssl stream - it's bad idea, you will get inifinite waiting for server closing ssl. Close socket directly with no worries
        getStream()->socket().shutdown(tcp::socket::shutdown_both, ec);

        if (ec && !isIgnoredError(ec) && ec != boost::system::errc::not_connected) {
            fail(ec, "shutdown");
            return;
        }

        if(m_progressFunc) {
            uint64_t total_len = m_response.content_length().value_or(0ULL);
            if (total_len == 0) {
                m_progressFunc(0ULL, 0ULL, 0.0);
            } else {
                m_progressFunc(total_len, total_len, 1.0);
            }
        }

        if (m_successFunc) {
            m_successFunc(m_response.release(), bytesTransferred);
        }
        // here everything gracefully closed
    } else {
        if(m_progressFunc) {
            uint64_t total_len = m_response.content_length().value_or(0ULL);
            uint64_t remain = m_response.content_length_remaining().value_or(0ULL);
            if(total_len == 0) {
                m_progressFunc(0ULL, 0ULL, 0.0);
            } else {
                const uint64_t loaded = total_len - remain;
                const double progress = (double)loaded / (double)total_len;
                m_progressFunc(loaded, total_len, progress);
            }
        }

        readResponse();
    }
}

void httb::async_session::setVerbose(bool verbose) {
    m_verbose = verbose;
}

void httb::async_session::setOnProgress(httb::async_session::progress_func_t progress) {
    m_progressFunc = progress;
}

void httb::async_session::setConnectionTimeout(std::chrono::seconds timeout) {
    m_connectionTimeout = std::move(timeout);
}

void httb::async_session::setReadTimeout(std::chrono::seconds timeout) {
    m_readTimeout = std::move(timeout);
}

bool httb::async_session::isIgnoredError(boost::system::error_code ec) {
    return std::find(m_ignoredErrors.begin(), m_ignoredErrors.end(), ec) != m_ignoredErrors.end();
}

void httb::async_session::fail(boost::system::error_code ec, char const *where) {
    if (m_errorFunc) {
        m_errorFunc(ec, std::string(where));
    }
}
void httb::async_session::v(const std::string &tag, const std::string &msg) {
    if (!m_verbose) {
        return;
    }
    std::cout << tag << ": " << msg << std::endl;
}

boost::beast::tcp_stream *httb::async_session::getStream() {
    return m_requestRaw.isSSL() ? &m_streamSSL->next_layer() : m_streamRaw;
}
