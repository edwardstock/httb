/**
 * httb
 * HttpClient.h
 *
 * @author Eduard Maximovich <edward.vstock@gmail.com>
 * @link https://github.com/edwardstock
 */

#ifndef HTTB_CLIENT_H
#define HTTB_CLIENT_H

#include <string>
#include <iostream>
#include <istream>
#include <ostream>
#include <chrono>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

#include "response.h"
#include "request.h"

using namespace std::chrono_literals;
namespace net = boost::asio;

namespace httb {

class client_base {
public:
    client_base();
    virtual ~client_base();

    /// \brief Set verbosity mode for curl
    /// \param enable
    /// \return chain
    void setEnableVerbose(bool enable, std::ostream *os = &std::cout);

    /// \brief Set connection timeout
    /// \param timeoutSeconds Long seconds
    /// \return chain
    void setConnectionTimeout(size_t connectionSeconds);

    /// \brief Set reading timeout
    /// \param readSeconds
    void setReadTimeout(size_t readSeconds);

    /// \brief Set how to react on 302 code
    /// \param followRedirects
    /// \return chain
    void setFollowRedirects(bool followRedirects, int maxBounces = 5);

    int getMaxRedirectBounces() const;
    bool isEnableVerbose() const;
    bool isFollowRedirects() const;

protected:
    std::ostream *m_verboseOutput;
    int m_maxRedirectBounces = 5;
    boost::asio::ssl::context m_ctx;
    bool m_followRedirects = true;
    bool m_verbose = false;
    std::chrono::seconds m_connectionTimeout = 30s;
    std::chrono::seconds m_readTimeout = 30s;

    void scanDir(const boost::filesystem::path &path, std::unordered_map<std::string, std::string> &map);
    void loadRootCertificates(boost::asio::ssl::context &ctx, boost::system::error_code &ec);
    inline void loadRootCertificates(boost::asio::ssl::context& ctx);
    inline httb::response boostErrorToResponseError(httb::response &&in, boost::system::error_code ec);
    inline httb::response boostErrorToResponseError(httb::response &&in, const boost::system::system_error &e);
};

/// \brief Simple Http Client based on low level http library boost beast
class client: public httb::client_base {
 public:
    using OnResponse = std::function<void(httb::response)>;

    client();
    ~client() override;
    /// \brief Make request using request
    /// \param request wss::web::Request
    /// \return wss::web::Response
    httb::response executeBlocking(const httb::request &request);

    /// \brief ASIO-based async blocking execution using io_context
    /// \param request
    /// \param cb
    void execute(const httb::request &request, const OnResponse &cb);

    void executeInContext(boost::asio::io_context &ioc, const httb::request &request, const OnResponse &cb);

};

}

#endif //HTTB_CLIENT_H
