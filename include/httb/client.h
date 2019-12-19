/**
 * httb
 * HttpClient.h
 *
 * @author Eduard Maximovich <edward.vstock@gmail.com>
 * @link https://github.com/edwardstock
 */

#ifndef HTTB_CLIENT_H
#define HTTB_CLIENT_H

#include "httb/httb_config.h"
#include "request.h"
#include "response.h"
#include "types.h"

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

using namespace std::chrono_literals;
namespace net = boost::asio;

namespace httb {

/// \brief Response callback (success and failed)
using response_func_t = std::function<void(httb::response)>;

class HTTB_API client_base {
public:
    client_base();
    virtual ~client_base();

    /// \brief Set verbosity mode for curl
    /// \param enable
    /// \return chain
    void set_verbose(bool enable, std::ostream* os = &std::cout);

    /// \brief Set connection timeout
    /// \param timeoutSeconds Long seconds
    /// \return chain
    void set_connection_timeout(size_t connectionSeconds);

    /// \brief Set reading timeout
    /// \param readSeconds
    void set_read_timeout(size_t readSeconds);

    /// \brief Set how to react on 302 code
    /// \param followRedirects
    /// \return chain
    void set_follow_redirects(bool followRedirects, int maxBounces = 5);

    /// \brief Get limit of bounces to redirect
    /// \return integer value
    int get_max_redirect_bounces() const;
    bool get_follow_redirects() const;

protected:
    std::ostream* m_ostream;
    int m_max_redirect_bounces = 5;
    net::ssl::context m_ctx;
    bool m_follow_redirects = true;
    bool m_verbose = false;
    std::chrono::seconds m_conn_timeout = 30s;
    std::chrono::seconds m_read_timeout = 30s;
};

/// \brief Simple Http Client based on low level http library boost beast
class HTTB_API client : public httb::client_base {
public:
    client();
    ~client() override;

    /// \brief Make request using request
    /// \param request wss::web::Request
    /// \return wss::web::Response
    virtual httb::response execute_blocking(const request& request);

    /// \brief ASIO-based async blocking execution using io_context
    /// \param request
    /// \param cb
    virtual void execute(const request& request, const response_func_t& cb, const progress_func_t& onProgress = nullptr);

    /// \brief ASIO-based async blocking execution using custom io_context
    /// \param ioc net::io_context
    /// \param request your request
    /// \param cb response callback
    /// \param onProgress progress callback
    virtual void execute_in_context(net::io_context& ioc, const request& request, const response_func_t& cb, const progress_func_t& onProgress = nullptr);
};

class HTTB_API batch_request {
public:
    /// \brief Each request result callback
    using on_response_each_func = httb::response_func_t;
    /// \brief All passed request result callback
    using on_response_all_func = std::function<void(std::vector<response>)>;

    batch_request();
    batch_request(uint32_t concurrency);

    /// \brief Add request to queue (move ctor)
    /// \param req movable request
    /// \return self
    const httb::batch_request& add(httb::request&& req);

    /// \brief Add request to queue
    /// \param req request
    /// \return self
    const httb::batch_request& add(const httb::request& req);

    void run_all(const on_response_each_func& cb);
    void run_all(const on_response_all_func& cb);

private:
    uint32_t m_concurrency;
    httb::context m_ctx;
    httb::client m_client;
    std::deque<httb::request> m_requests;
};

} // namespace httb

#endif //HTTB_CLIENT_H
