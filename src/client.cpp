/**
 * wsserver_standalone
 * HttpClient.cpp
 *
 * @author Eduard Maximovich <edward.vstock@gmail.com>
 * @link https://github.com/edwardstock
 */

#include <thread>
#include <future>
#include <toolboxpp.h>
#include "httb/client.h"
#include "helpers.hpp"
#include "httb/request.h"
#include <boost/beast/core/file.hpp>

// CLIENT
httb::client_base::client_base():
    m_verboseOutput(&std::cout),
    m_ctx(boost::asio::ssl::context::sslv23_client) {
    loadRootCertificates(m_ctx);
}
httb::client_base::~client_base() {

}
void httb::client_base::setEnableVerbose(bool enable, std::ostream *os) {
    m_verbose = enable;
    m_verboseOutput = os;
}

void httb::client_base::setConnectionTimeout(long timeoutSeconds) {
    m_connectionTimeout = timeoutSeconds;
}

void httb::client_base::setFollowRedirects(bool followRedirects, int maxBounces) {
    m_followRedirects = followRedirects;
    m_maxRedirectBounces = maxBounces;
}

bool httb::client_base::isEnableVerbose() const {
    return m_verbose;
}
long httb::client_base::getTimeout() const {
    return m_connectionTimeout;
}
int httb::client_base::getMaxRedirectBounces() const {
    return m_maxRedirectBounces;
}
bool httb::client_base::isFollowRedirects() const {
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

void httb::client_base::loadRootCertificates(boost::asio::ssl::context &ctx, boost::system::error_code &ec) {
    std::vector<std::string> certPaths = {
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
        scanDir(fs::path(path), certFiles);
    }

    for (auto &iter: certFiles) {
        ctx.add_certificate_authority(
            boost::asio::buffer(iter.second.data(), iter.second.size()), ec);
    }
}

void httb::client_base::loadRootCertificates(boost::asio::ssl::context &ctx) {
    boost::system::error_code ec;
    httb::client_base::loadRootCertificates(ctx, ec);
    if (ec)
        throw boost::system::system_error{ec};
}

httb::client::client() : client_base() {
}
httb::client::~client() {
}

httb::response httb::client::execute(const httb::request &request) {
    httb::response resp;

    using tcp = boost::asio::ip::tcp;
    namespace http = boost::beast::http;
    namespace ssl = boost::asio::ssl;

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver{ioc};
    // Look up the domain name
    const auto results = resolver.resolve(request.getHost(), request.getPortString());

    boost::system::error_code ec;

    auto req = request.createBeastRequest();

    if(m_verbose && m_verboseOutput) {
        std::stringstream verboseStream;
        verboseStream << req;
        std::string verboseString = verboseStream.str();

        if(verboseStream.str().size() >= 1024) {
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

    if (request.isSSL()) {
        ssl::stream<tcp::socket> stream{ioc, m_ctx};
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(stream.native_handle(), request.getHost().c_str())) {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            throw boost::system::system_error{ec};
        }

        // Make the connection on the IP address we get from a lookup
        boost::asio::connect(stream.next_layer(), results.begin(), results.end());

        // Perform the SSL handshake
        stream.handshake(ssl::stream_base::client);

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // Receive the HTTP response
        http::read(stream, buffer, res);

        stream.shutdown(ec);

        if (ec == boost::asio::error::eof) {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec.assign(0, ec.category());
        }
    } else {
        tcp::socket socket{ioc};

        // Make the connection on the IP address we get from a lookup
        boost::asio::connect(socket, results.begin(), results.end());

        // Send the HTTP request to the remote host
        http::write(socket, req);

        // Receive the HTTP response
        http::read(socket, buffer, res);

        socket.shutdown(tcp::socket::shutdown_both, ec);
    }

    s = boost::beast::buffers_to_string(res.body().data());

    if (ec && ec != boost::system::errc::not_connected)
        throw boost::system::system_error{ec};

    resp.status = res.result();
    resp.statusCode = static_cast<typename std::underlying_type<httb::response::http_status>::type>(res.result());
    resp.statusMessage = res.reason().to_string();
    for (auto const &field: res) {
        resp.addHeader(field.name_string(), field.value());
    }
    resp.setBody(std::move(s));

    res.body().consume(res.body().size());


    if(m_followRedirects) {
        int redirectBounces = 0;
        while(redirectBounces < m_maxRedirectBounces && (
            resp.status == httb::response::http_status::moved_permanently ||
                resp.status == httb::response::http_status::found ||
                resp.status == httb::response::http_status::temporary_redirect ||
                resp.status == httb::response::http_status::permanent_redirect
        )) {

            if(!resp.hasHeader("location")) {
                // throw std::runtime_error("Unable to redirect: Location header is undefined");
                return resp;
            }

            // copy request
            auto redirectRequest = request;

            // set to new request Location url
            redirectRequest.parseUrl(resp.getHeader("location"));

            // overwrite current response with new request
            resp = execute(redirectRequest);

            // and repeat while we don't get 2xx code or redirect bounces reaches 5 times
            redirectBounces++;
        }
    }

    return resp;
}

void httb::client::executeAsync(httb::request request, std::function<void(httb::response)> cb) {
    auto res = std::async(std::launch::async, [this, req = std::move(request), c = std::move(cb)](){
      auto res = execute(req);
      if(c) {
          c(res);
      }
    });
}
