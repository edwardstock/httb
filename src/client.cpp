/**
 * httb
 * client.cpp
 *
 * @author Eduard Maximovich <edward.vstock@gmail.com>
 * @link https://github.com/edwardstock
 */

#include "httb/client.h"

#include "async_session.h"
#include "httb/request.h"
#include "utils.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/version.hpp>
#include <thread>
#include <toolbox/io.h>
#include <toolbox/strings.hpp>
#include <type_traits>

httb::client_base::client_base()
    : m_ostream(&std::cout),
      m_ctx(boost::asio::ssl::context::sslv23_client) {
    //    load_root_certs(m_ctx);
}
httb::client_base::~client_base() {
}
void httb::client_base::set_verbose(bool enable, std::ostream* os) {
    m_verbose = enable;
    m_ostream = os;
}

void httb::client_base::set_connection_timeout(size_t connectionSeconds) {
    m_conn_timeout = std::chrono::seconds(connectionSeconds);
}

void httb::client_base::set_read_timeout(size_t readSeconds) {
    m_read_timeout = std::chrono::seconds(readSeconds);
}

void httb::client_base::set_follow_redirects(bool followRedirects, int maxBounces) {
    m_follow_redirects = followRedirects;
    m_max_redirect_bounces = maxBounces;
}

int httb::client_base::get_max_redirect_bounces() const {
    return m_max_redirect_bounces;
}
bool httb::client_base::get_follow_redirects() const {
    return m_follow_redirects;
}

httb::client::client()
    : client_base() {
}
httb::client::~client() {
}

httb::response httb::client::execute_blocking(const httb::request& request) {
    httb::response resp;
    boost::system::error_code ec;

    using tcp = boost::asio::ip::tcp;
    namespace http = boost::beast::http;
    namespace ssl = boost::asio::ssl;

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver{ioc};
    // Look up the domain name

    const auto results = resolver.resolve(request.get_host(), request.get_port_str(), ec);

    if (ec) {
        return boost_err_to_rep_err(std::move(resp), ec);
    }

    auto req = request.to_beast_request();

    if (m_verbose && m_ostream) {
        std::stringstream verboseStream;
        verboseStream << req;
        std::string verboseString = verboseStream.str();

        if (verboseStream.str().size() >= 1024) {
            verboseString = verboseString.substr(0, 1023);
            verboseString += " ...(too big for output)";
        }
        (*(m_ostream)) << verboseString << std::endl;
    }

    // This buffer is used for reading and must be persisted
    boost::beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    std::string s;

    try {
        if (request.is_ssl()) {
            beast::ssl_stream<beast::tcp_stream> stream{ioc, m_ctx};
            // Set SNI Hostname (many hosts need this to handshake successfully)
            if (!SSL_set_tlsext_host_name(stream.native_handle(), request.get_host().c_str())) {
                boost::system::error_code
                    ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
                return boost_err_to_rep_err(std::move(resp), ec);
            }

            stream.next_layer().expires_after(std::chrono::seconds(m_conn_timeout));

            // Make the connection on the IP address we get from a lookup
            beast::get_lowest_layer(stream).connect(results);

            // Perform the SSL handshake
            stream.handshake(ssl::stream_base::client);

            // Send the HTTP request to the remote host
            http::write(stream, req, ec);

            stream.next_layer().expires_after(std::chrono::seconds(m_read_timeout));
            // Receive the HTTP response
            http::read(stream, buffer, res, ec);

            stream.next_layer().socket().shutdown(tcp::socket::shutdown_both, ec);

            if (ec == boost::asio::ssl::error::stream_truncated) {
                // it's just empty ssl response, not expected but not critical case
                ec.assign(0, ec.category());
            }

            if (ec == boost::asio::error::eof) {
                // Rationale:
                // https://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
                ec.assign(0, ec.category());
            }
        } else {
            beast::tcp_stream stream(ioc);
            // set connection timeout
            stream.expires_after(std::chrono::seconds(m_conn_timeout));
            // Make the connection on the IP address we get from a lookup
            beast::get_lowest_layer(stream).connect(results);
            stream.expires_never();

            // Send the HTTP request to the remote host
            http::write(stream, req, ec);

            // set read timeout
            stream.expires_after(std::chrono::seconds(m_read_timeout));
            // Receive the HTTP response
            http::read(stream, buffer, res, ec);

            stream.socket().shutdown(tcp::socket::shutdown_both);
        }
    } catch (const boost::system::system_error& e) {
        if (e.code() != boost::system::errc::not_connected) {
            return boost_err_to_rep_err(std::move(resp), e);
        }
    }

    s = boost::beast::buffers_to_string(res.body().data());

    if (ec && ec != boost::system::errc::not_connected) {
        return boost_err_to_rep_err(std::move(resp), ec);
    }

    resp.status = res.result();
    resp.code = static_cast<typename std::underlying_type<httb::response::http_status>::type>(res.result());
    resp.status_message = res.reason().to_string();
    for (auto const& field : res) {
        resp.add_header(field.name_string().to_string(), field.value().to_string());
    }
    resp.set_body(std::move(s));

    res.body().consume(res.body().size());

    if (m_follow_redirects) {
        int redirectBounces = 0;
        while (redirectBounces < m_max_redirect_bounces && (resp.status == httb::response::http_status::moved_permanently ||
                                                            resp.status == httb::response::http_status::found ||
                                                            resp.status == httb::response::http_status::temporary_redirect ||
                                                            resp.status == httb::response::http_status::permanent_redirect)) {

            if (!resp.has_header("location")) {
                return resp;
            }

            // copy request
            auto redirectRequest = request;

            // set to new request Location url
            redirectRequest.parse_url(resp.get_header_value("location"));

            // overwrite current response with new request
            resp = execute_blocking(redirectRequest);

            // and repeat while we don't get 2xx code or redirect bounces reaches 5 times
            redirectBounces++;
        }
    }

    return resp;
}

void httb::client::execute_in_context(boost::asio::io_context& ioc,
                                      const httb::request& request,
                                      const response_func_t& cb,
                                      const progress_func_t& onProgress) {
    std::shared_ptr<httb::async_session> session = std::make_shared<httb::async_session>(ioc, request, m_conn_timeout, m_read_timeout);
    session->set_verbose(m_verbose);
    session->set_on_progress_cb(onProgress);

    session->run(
        [cb](boost::system::error_code ec, const std::string& wtf) {
            httb::response resp;
            auto res = boost_err_to_rep_err(std::move(resp), ec);
            auto body = res.get_body();
            body += "::" + wtf;
            res.set_body(std::move(body));
            cb(res);
        },
        [this, cb, onProgress, &ioc, &request](httb::response_t&& result, size_t) {
            auto res = std::move(result);
            std::string s = boost::beast::buffers_to_string(res.body().data());
            httb::response resp;
            resp.set_body(std::move(s));
            resp.status = res.result();
            resp.code = static_cast<typename std::underlying_type<httb::response::http_status>::type>(res.result());
            resp.status_message = res.reason().to_string();
            for (auto const& field : res) {
                resp.add_header(field.name_string().to_string(), field.value().to_string());
            }

            res.body().consume(res.body().size());

            if (m_follow_redirects && (resp.status == httb::response::http_status::moved_permanently ||
                                       resp.status == httb::response::http_status::found ||
                                       resp.status == httb::response::http_status::temporary_redirect ||
                                       resp.status == httb::response::http_status::permanent_redirect)) {
                if (!resp.has_header("location")) {
                    cb(resp);
                    return;
                }

                // copy request
                auto redirectRequest = request;

                // set to new request Location url
                redirectRequest.parse_url(resp.get_header_value("location"));

                // overwrite current response with new request
                execute_in_context(ioc, (redirectRequest), cb, onProgress);
                return;
            }

            if (cb)
                cb(resp);
        });
}

void httb::client::execute(const httb::request& request,
                           const response_func_t& cb,
                           const progress_func_t& onProgress) {
    boost::asio::io_context ioc(2);
    execute_in_context(ioc, request, cb, onProgress);
    ioc.run();
}

httb::batch_request::batch_request()
    : m_concurrency(std::thread::hardware_concurrency()),
      m_ctx(m_concurrency) {
}

httb::batch_request::batch_request(uint32_t concurrency)
    : m_concurrency(concurrency),
      m_ctx(m_concurrency) {
}

const httb::batch_request& httb::batch_request::add(httb::request&& req) {
    m_requests.push_back(std::move(req));
    return *this;
}

const httb::batch_request& httb::batch_request::add(const httb::request& req) {
    m_requests.push_back(req);
    return *this;
}

void httb::batch_request::run_all(const httb::batch_request::on_response_each_func& cb) {
    while (!m_requests.empty()) {
        auto req = m_requests.back();
        m_requests.pop_back();
        m_client.execute_in_context(m_ctx, req, cb);
    }
    m_ctx.run();
}

void httb::batch_request::run_all(const httb::batch_request::on_response_all_func& cb) {
    std::vector<httb::response> out;
    out.reserve(m_requests.size());
    std::mutex resLock;

    while (!m_requests.empty()) {
        auto req = m_requests.back();
        m_requests.pop_back();
        m_client.execute_in_context(m_ctx, req, [&resLock, &out](httb::response result) {
            std::lock_guard<std::mutex> lock(resLock);
            out.push_back(std::move(result));
        });
    }
    m_ctx.run();

    cb(out);
}
