/*!
 * wsserver.
 * Response.h
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef WSSERVER_RESPONSE_H
#define WSSERVER_RESPONSE_H

#include <string>
#include <boost/beast.hpp>
#include "httb/io_container.h"
//#include "httb/client.h"

namespace httb {
class response : public httb::io_container {
public:
    friend class client;
    using http_status = boost::beast::http::status;

    int statusCode;
    http_status status;

    std::string statusMessage;
    std::string data;
    std::string _headersBuffer;

//    /// \brief Return json data from body, empty object if can't parse
//    /// \return
//    nlohmann::json parseJsonBody() const;

    /// \brief Return map of POST body form-url-encoded data
    /// \return
    kv_vector parseFormUrlEncode() const;

    /// \brief Print response data to std::cout
    void dump() const;

    std::string getBody() const override;
    const char *getBodyC() const override;
    bool hasBody() const override;
    size_t getBodySize() const override;

    void setBody(const std::string &body) override;
    void setBody(std::string &&body) override;

    /// \brief Check response status  200 <= code < 400
    /// \return
    bool isSuccess() const;
};
}

#endif //WSSERVER_RESPONSE_H
