/**
 * wsserver_standalone
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
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "response.h"
#include "request.h"

namespace httb {

class client_base {
public:
    client_base();
    virtual ~client_base();

    /// \brief Set verbosity mode for curl
    /// \param enable
    /// \return chain
    void setEnableVerbose(bool enable, std::ostream *os = &std::cout);

    /// \brief Set curl connection timeout
    /// \param timeoutSeconds Long seconds
    /// \return chain
    void setConnectionTimeout(long timeoutSeconds);

    /// \brief Set how to react on 302 code
    /// \param followRedirects
    /// \return chain
    void setFollowRedirects(bool followRedirects, int maxBounces = 5);

    long getTimeout() const;
    int getMaxRedirectBounces() const;
    bool isEnableVerbose() const;
    bool isFollowRedirects() const;

protected:
    std::ostream *m_verboseOutput;
    int m_maxRedirectBounces = 5;
    boost::asio::ssl::context m_ctx;
    bool m_followRedirects = false;
    bool m_verbose = false;
    long m_connectionTimeout = 10L;

    void scanDir(const boost::filesystem::path &path, std::unordered_map<std::string, std::string> &map);
    void loadRootCertificates(boost::asio::ssl::context &ctx, boost::system::error_code &ec);
    inline void loadRootCertificates(boost::asio::ssl::context& ctx);
};

/// \brief Simple Http Client based on low level http library boost beast
class client: public httb::client_base {
 public:
    client();
    ~client() override;
    /// \brief Make request using request
    /// \param request wss::web::Request
    /// \return wss::web::Response
    httb::response execute(const httb::request &request);

    /// \brief Simple async execution with std::future
    /// \param request
    /// \param cb
    void executeAsync(httb::request request, std::function<void(httb::response)> cb);

};

}

#endif //HTTB_CLIENT_H
