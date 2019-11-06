/**
 * httb
 * client.cpp
 *
 * @author Eduard Maximovich <edward.vstock@gmail.com>
 * @link https://github.com/edwardstock
 */

#include <thread>
#include <future>
#include <toolboxpp.hpp>
#include <type_traits>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/version.hpp>

#include "httb/client.h"
#include "httb/helpers.hpp"
#include "httb/request.h"
#include "httb/timer.h"
#include "httb/async_session.h"

httb::client_base::client_base() :
    m_verboseOutput(&std::cout),
    m_ctx(boost::asio::ssl::context::sslv23_client) {
    loadRootCerts(m_ctx);
}
httb::client_base::~client_base() {

}
void httb::client_base::setVerbose(bool enable, std::ostream *os) {
    m_verbose = enable;
    m_verboseOutput = os;
}

void httb::client_base::setConnectionTimeout(size_t connectionSeconds) {
    m_connectionTimeout = std::chrono::seconds(connectionSeconds);
}

void httb::client_base::setReadTimeout(size_t readSeconds) {
    m_readTimeout = std::chrono::seconds(readSeconds);
}

void httb::client_base::setFollowRedirects(bool followRedirects, int maxBounces) {
    m_followRedirects = followRedirects;
    m_maxRedirectBounces = maxBounces;
}

bool httb::client_base::getVerbose() const {
    return m_verbose;
}

int httb::client_base::getMaxRedirectBounces() const {
    return m_maxRedirectBounces;
}
bool httb::client_base::getFollowRedirects() const {
    return m_followRedirects;
}

void httb::client_base::scanDir(const boost::filesystem::path &path,
                                std::unordered_map<std::string, std::string> &map) {
    namespace fs = boost::filesystem;

    if (!fs::exists(path)) {
        return;
    }

    if (fs::is_regular_file(path)) {
        const std::string ext = path.extension().generic_string();
        if (toolboxpp::strings::equalsIgnoreCase(".crt", ext) || toolboxpp::strings::equalsIgnoreCase(".pem", ext)) {
            map[path.generic_string()] = toolboxpp::fs::readFile(path.generic_string());
        }
    } else if (fs::is_directory(path)) {
        for (fs::directory_entry &x : fs::directory_iterator(path)) {
            scanDir(x.path(), map);
        }
    } else {
        // irregular for or directory
    }
}

void httb::client_base::loadRootCerts(boost::asio::ssl::context &ctx, boost::system::error_code &ec) {
    const std::vector<std::string> certPaths = {
        "/etc/ssl/certs/ca-certificates.crt",                // Debian/Ubuntu/Gentoo etc.
        "/etc/pki/tls/certs",                                // Fedora/RHEL
        "/etc/pki/tls/certs/ca-bundle.crt",                  // Fedora/RHEL 6
        "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", // CentOS/RHEL 7
        "/etc/ssl/ca-bundle.pem",                            // OpenSUSE
        "/etc/pki/tls/cacert.pem",                           // OpenELEC
        "/etc/ssl/certs",                                    // SLES10/SLES11, https://golang.org/issue/12139
        "/etc/ssl/cert.pem",                                 // macOS Common
        "/usr/local/etc/openssl/certs",                      // macOS OpenSSL
        "/usr/local/etc/openssl@1.1/certs",                  // macOS OpenSSL 1.1
        "/system/etc/security/cacerts",                      // Android
        "/usr/local/share/certs",                            // FreeBSD
        "/etc/openssl/certs",                                // NetBSD
    };

    namespace fs = boost::filesystem;
    std::unordered_map<std::string, std::string> certFiles;
    for (const auto &path: certPaths) {
        try {
            scanDir(fs::path(path), certFiles);
        } catch (const std::exception &e) {

        }

    }

    for (auto &iter: certFiles) {
        ctx.add_certificate_authority(
            boost::asio::buffer(iter.second.data(), iter.second.size()), ec);
    }
}

void httb::client_base::loadRootCerts(boost::asio::ssl::context &ctx) {
    boost::system::error_code ec;
    httb::client_base::loadRootCerts(ctx, ec);
    if (ec)
        throw boost::system::system_error{ec};
}

httb::response httb::client_base::boostErrorToResponseError(httb::response &&in, boost::system::error_code ec) {
    httb::response out = std::move(in);
    out.statusCode = httb::response::INTERNAL_ERROR_OFFSET + ec.value();
    std::stringstream errorStream;
    errorStream << ec.category().name() << "[" << out.statusCode << "]" << ": " << ec.message();
    out.setBody(errorStream.str());

    return out;
}

httb::response httb::client_base::boostErrorToResponseError(httb::response &&in,
                                                            const boost::system::system_error &e) {
    return boostErrorToResponseError(std::move(in), e.code());
}

httb::client::client() : client_base() {
}
httb::client::~client() {
}

httb::response httb::client::executeBlocking(const httb::request &request) {
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

    const auto results = resolver.resolve(request.getHost(), request.getPortString(), ec);

    if (ec) {
        return boostErrorToResponseError(std::move(resp), ec);
    }

    auto req = request.createBeastRequest();

    if (m_verbose && m_verboseOutput) {
        std::stringstream verboseStream;
        verboseStream << req;
        std::string verboseString = verboseStream.str();

        if (verboseStream.str().size() >= 1024) {
            verboseString = verboseString.substr(0, 1023);
            verboseString += " ...(too big for output)";
        }
        (*(m_verboseOutput)) << verboseString << std::endl;
    }


    // This buffer is used for reading and must be persisted
    boost::beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    std::string s;

    try {
        if (request.isSSL()) {
            ssl::stream<tcp::socket> stream{ioc, m_ctx};
            // Set SNI Hostname (many hosts need this to handshake successfully)
            if (!SSL_set_tlsext_host_name(stream.native_handle(), request.getHost().c_str())) {
                boost::system::error_code
                    ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
                return boostErrorToResponseError(std::move(resp), ec);
            }

            // Make the connection on the IP address we get from a lookup
            boost::asio::connect(stream.next_layer(), results.begin(), results.end());

            // Perform the SSL handshake
            stream.handshake(ssl::stream_base::client);

            // Send the HTTP request to the remote host
            http::write(stream, req, ec);

            // Receive the HTTP response
            http::read(stream, buffer, res, ec);

            {
                timer t(m_readTimeout, [&stream] {
                  stream.lowest_layer().shutdown(boost::asio::socket_base::shutdown_both);
                });
                stream.shutdown(ec);
                t.cancel();
            }

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
            tcp::socket socket{ioc};

            // Make the connection on the IP address we get from a lookup
            boost::asio::connect(socket, results.begin(), results.end());

            // Send the HTTP request to the remote host
            http::write(socket, req, ec);

            // Receive the HTTP response
            http::read(socket, buffer, res, ec);

            socket.shutdown(tcp::socket::shutdown_both);
        }
    } catch (const boost::system::system_error &e) {
        if (e.code() != boost::system::errc::not_connected) {
            return boostErrorToResponseError(std::move(resp), e);
        }
    }

    s = boost::beast::buffers_to_string(res.body().data());

    if (ec && ec != boost::system::errc::not_connected) {
        return boostErrorToResponseError(std::move(resp), ec);
    }

    resp.status = res.result();
    resp.statusCode = static_cast<typename std::underlying_type<httb::response::http_status>::type>(res.result());
    resp.statusMessage = res.reason().to_string();
    for (auto const &field: res) {
        resp.addHeader(field.name_string(), field.value());
    }
    resp.setBody(std::move(s));

    res.body().consume(res.body().size());

    if (m_followRedirects) {
        int redirectBounces = 0;
        while (redirectBounces < m_maxRedirectBounces && (
            resp.status == httb::response::http_status::moved_permanently ||
                resp.status == httb::response::http_status::found ||
                resp.status == httb::response::http_status::temporary_redirect ||
                resp.status == httb::response::http_status::permanent_redirect
        )) {

            if (!resp.containsHeader("location")) {
                return resp;
            }

            // copy request
            auto redirectRequest = request;

            // set to new request Location url
            redirectRequest.parseUrl(resp.getHeader("location"));

            // overwrite current response with new request
            resp = executeBlocking(redirectRequest);

            // and repeat while we don't get 2xx code or redirect bounces reaches 5 times
            redirectBounces++;
        }
    }

    return resp;
}

void httb::client::executeInContext(boost::asio::io_context &ioc,
                                    const httb::request &request,
                                    const response_func_t &cb,
                                    const progress_func_t &onProgress) {
    std::shared_ptr<httb::async_session> session = std::make_shared<httb::async_session>(ioc, (request));
    session->setVerbose(m_verbose);

    session->setOnProgress(onProgress);
    session->run(
        [this, cb](boost::system::error_code ec, std::string when) {
          httb::response resp;
          auto res = boostErrorToResponseError(std::move(resp), ec);
          auto body = res.getBody();
          body += "::" + when;
          res.setBody(std::move(body));
          cb(res);
        },
        [this, cb, onProgress, &ioc, &request](httb::response_t &&result, size_t) {
          auto res = std::move(result);
          std::string s = boost::beast::buffers_to_string(res.body().data());
          httb::response resp;
          resp.setBody(std::move(s));
          resp.status = res.result();
          resp.statusCode = static_cast<typename std::underlying_type<httb::response::http_status>::type>(res.result());
          resp.statusMessage = res.reason().to_string();
          for (auto const &field: res) {
              resp.addHeader(field.name_string(), field.value());
          }

          res.body().consume(res.body().size());

          if (m_followRedirects && (resp.status == httb::response::http_status::moved_permanently ||
              resp.status == httb::response::http_status::found ||
              resp.status == httb::response::http_status::temporary_redirect ||
              resp.status == httb::response::http_status::permanent_redirect)
              ) {
              if (!resp.containsHeader("location")) {
                  cb(resp);
                  return;
              }

              // copy request
              auto redirectRequest = request;

              // set to new request Location url
              redirectRequest.parseUrl(resp.getHeader("location"));

              // overwrite current response with new request
              executeInContext(ioc, (redirectRequest), cb, onProgress);
              return;

          }

          if (cb) cb(resp);
        });
}

void httb::client::execute(const httb::request &request,
                           const response_func_t &cb,
                           const progress_func_t &onProgress) {
    boost::asio::io_context ioc(2);
    executeInContext(ioc, request, cb, onProgress);
    ioc.run();
}


httb::batch_request::batch_request():
    m_concurrency(std::thread::hardware_concurrency()),
    m_ctx(m_concurrency) {
}

httb::batch_request::batch_request(uint32_t concurrency):
    m_concurrency(concurrency),
    m_ctx(m_concurrency) {

}

const httb::batch_request &httb::batch_request::add(httb::request &&req) {
    m_requests.push_back(std::move(req));
    return *this;
}

const httb::batch_request &httb::batch_request::add(const httb::request &req) {
    m_requests.push_back(req);
    return *this;
}

void httb::batch_request::runEach(const httb::batch_request::on_response_each_func &cb) {
    while(!m_requests.empty()) {
        auto req = m_requests.back();
        m_requests.pop_back();
        m_client.executeInContext(m_ctx, req, cb);
    }
    m_ctx.run();
}

void httb::batch_request::runAll(const httb::batch_request::on_response_all_func &cb) {
    std::vector<httb::response> out;
    out.reserve(m_requests.size());
    std::mutex resLock;

    while(!m_requests.empty()) {
        auto req = m_requests.back();
        m_requests.pop_back();
        m_client.executeInContext(m_ctx, req, [&resLock, &out](httb::response result) {
            std::lock_guard<std::mutex> lock(resLock);
            out.push_back(std::move(result));
        });
    }
    m_ctx.run();

    cb(out);
}
