/*!
 * wsserver.
 * Request.h
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_REQUEST_H
#define HTTB_REQUEST_H

#include <sstream>
#include <memory>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include "httb/body.h"
#include "httb/body_multipart.h"
#include "httb/body_form_urlencoded.h"
#include "httb/body_string.h"
#include "httb/io_container.h"

namespace httb {

class base_request : public httb::io_container {
public:
    using status = boost::beast::http::status;
    /// \brief Http methods
    using method = boost::beast::http::verb;

    base_request();
    explicit base_request(const std::string &url);
    explicit base_request(const std::string &url, uint16_t port);
    base_request(const std::string &url, base_request::method method);

    /// \brief parse query string to vector<KeyValue>. String must not contains hostname or protocol, only query string.
    /// Example: ?id=1&param=2&someKey=3
    /// Warning! Keys represented as arrays, will not be recognized as arrays, they will stored as multiple values, of one keys,
    /// and if you will try to get param only by key using getParam(const std::string&), method will return only first found value, not all.
    /// \param queryString
    void parseParamsString(const std::string &queryString);

    /// \brief Convert string method name to wss::web::Request::Method
    /// \param methodName case insensitive string
    /// \return if method string will not be recognized, method will return wss::web::Request::Method::GET
    static method methodFromString(const std::string &methodName);

    /// \brief Convert method to uppercase string
    /// \param methodName
    /// \return http method name
    static std::string methodToString(method methodName);

    /// \brief Set request http method
    /// \param method
    void setMethod(method method);

    /// \brief Add query param key-value wss::web::KeyValue
    /// \param keyValue pair of strings
    void addParam(kv &&keyValue);

    /// \brief Make request using SSL
    /// \param useSSL
    void useSSL(bool useSSL);

    /// \brief Return existed url
    /// \return url string or empty string if did not set
    std::string getUrl() const;

    /// \brief Return hostname
    /// \return pure hostname without protocol and port: google.com, facebook.com etc
    std::string getHost() const;

    /// \brief Retrun port number
    /// \return Default: 80 if url has http://, if url protocol https without explicit port number - 443,
    /// otherwise - custom port number
    uint16_t getPort() const;

    /// \brief Return port number as string
    /// \return the same but string value
    std::string getPortString() const;
    /**
     * \brief Protocol name
     * \return simple name url started from: http, ftp, or what was passed
     */
    std::string getProto() const;

    /// \brief Url path (not a query params!)
    /// \return for example: "search" for url "https://google.com/search?q=1&c=2.0&etc=bla"
    std::string getPath() const;

    /// \brief Return Http method name
    /// \return
    method getMethod() const;

    /// \brief Whether call will be with requested with ssl stream or not
    /// \return true if ssl is used
    bool isSSL() const;

    /// \brief Check for at least one query parameter has set
    /// \return
    bool hasParams() const;

    /// \brief Check for parameter exists
    /// \param key query parameter name
    /// \param icase search case sensititvity
    /// \return true if parameter exists
    bool hasParam(const std::string &key, bool icase = true) const;

    /// \brief Return value of query parameter
    /// \param key query parameter name
    /// \param icase search case sensititvity
    /// \return empty string of parameter did not set
    std::string getParam(const std::string &key, bool icase = true) const;

    /// \brief Return multiple entries if param is an array.
    /// For example k[]=k&m[]=k&m=3, will return:
    /// std::vector<std::string> out(3);
    /// \param key
    /// \param icase
    /// \return vector of string pairs
    std::vector<std::string> getParamArray(const std::string &key, bool icase = true) const;

    /// \brief Build passed url with query parameters
    /// \return url with parameters. if url did not set, will return empty string without parameters
    std::string getParamsString() const;

    /// \brief Return copy of passed parameters
    /// \return simple vector with pairs of strings
    kv_vector getParams() const;

    // Use with carefully
    void parseUrl(const std::string &url);

private:
    bool m_ssl;
    boost::beast::http::verb m_method;
    std::string m_proto;
    std::string m_host;
    std::string m_port;
    std::string m_path;
    /// \brief like multimap but vector of pairs
    kv_vector m_params;
};

class request: public httb::base_request {
public:
    request() : base_request() { }
    explicit request(const std::string &url) : base_request(url) { }
    explicit request(const std::string &url, uint16_t port) : base_request(url, port) { }
    request(const std::string &url, base_request::method method) : base_request(url, method) { }

    boost::beast::http::request<boost::beast::http::string_body> createBeastRequest() const;

    void setBody(const std::string &body) override;
    void setBody(std::string &&body) override;
    void setBody(const httb::request_body &body);
    void setBody(httb::request_body &&body);
};

}

#endif //HTTB_REQUEST_H
