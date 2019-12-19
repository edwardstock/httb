/*!
 * wsserver.
 * Response.h
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_RESPONSE_H
#define HTTB_RESPONSE_H

#include "httb/io_container.h"
#include "types.h"

#include <boost/beast/http/status.hpp>
#include <string>

namespace httb {

class HTTB_API response : public httb::io_container {
public:
    static const int INTERNAL_ERROR_OFFSET = 1000;
    friend class client;
    using http_status = boost::beast::http::status;

    response();
    virtual ~response() = default;

    /// \brief Return map of POST body form-url-encoded data
    /// \return
    [[nodiscard]] kv_vector parse_form_url_encode() const;

    /// \brief Print response data to std::cout
    void dump() const;

    std::string get_body() const override;
    std::string get_body(bool clear);
    const char* get_body_c() const override;
    bool has_body() const override;
    size_t get_body_size() const override;

    void set_body(const std::string& body) override;
    void set_body(std::string&& body) override;

    /// \brief Check response status  200 <= code < 400
    /// \return
    bool success() const;
    bool is_internal_error();

    operator bool() const noexcept;

    template<typename T>
    response& operator<<(const T& body) {
        std::stringstream ss;
        ss << data;
        ss << body;
        data = ss.str();
        return *this;
    }

    int code;
    http_status status;

    std::string status_message;
    std::string data;
};

} // namespace httb

HTTB_API std::ostream& operator<<(std::ostream& os, const httb::response& resp);

#endif //HTTB_RESPONSE_H
