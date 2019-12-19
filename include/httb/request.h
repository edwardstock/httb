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

#include "httb/body.h"
#include "httb/body_form_urlencoded.h"
#include "httb/body_multipart.h"
#include "httb/body_string.h"
#include "httb/io_container.h"

#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/optional.hpp>
#include <memory>
#include <queue>
#include <sstream>
#include <unordered_map>

namespace httb {

class HTTB_API base_request : public httb::io_container {
public:
    using status = boost::beast::http::status;
    /// \brief Http methods
    using method = boost::beast::http::verb;

    base_request();
    explicit base_request(const std::string& url);
    explicit base_request(const std::string& url, uint16_t port);
    base_request(const std::string& url, base_request::method method);
    virtual ~base_request() = default;

    /// \brief Convert string method name to wss::web::Request::Method
    /// \param method_name case insensitive string
    /// \return if method string will not be recognized, method will return wss::web::Request::Method::GET
    static method method_from_string(const std::string& method_name);

    /// \brief Convert method to uppercase string
    /// \param methodName
    /// \return http method name
    static std::string method_to_string(method methodName);

    /// \brief Set request http method
    /// \param method
    void set_method(method method);

    /// \brief Explicitly mark to use ssl stream, event if parsed url does not have https protocol
    /// \param use
    void use_ssl(bool use);

    /// \brief Return existed url
    /// \return url string or empty string if did not set
    std::string get_url() const;

    /// \brief Return hostname
    /// \return pure hostname without protocol and port: google.com, facebook.com etc
    std::string get_host() const;

    /// \brief Set hostname for url
    /// \param hostname example: google.com
    void set_host(const std::string& hostname);

    /// \brief Retrun port number
    /// \return Default: 80 if url has http://, if url protocol https without explicit port number - 443,
    /// otherwise - custom port number
    uint16_t get_port() const;

    /// \brief Set port number for url
    /// \param portNumber 16 bit value, range is 0 - 65535
    void set_port(uint16_t portNumber);

    /// \brief Return port number as string
    /// \return the same but string value
    std::string get_port_str() const;

    /// \brief Protocol name
    /// \return simple name url started from: http, ftp, or what was passed
    std::string get_proto_name() const;

    /// \brief Set protocol name, like http or https or ftp
    /// \param protocolName
    void set_proto_name(const std::string& protocolName);

    /// \brief Url path (not a query params!)
    /// \return for example: "search" for url "https://google.com/search"
    std::string get_path() const;

    /// \brief Url path with prepared query params
    /// \return for example: "search" for url "https://google.com/search?q=1&c=2.0&etc=bla"
    std::string get_path_with_query() const;

    /// \brief Set url path (without query!)
    /// \param path for example: "/api/v1/get-my-money" for url "https://google.com/api/v1/get-my-money"
    void set_path(const std::string& path);

    /// \brief Add to existing path new path or just set it
    /// \param path for example: was "/api/v1/", your'e adding "/user/create", result will: /api/v1/user/create
    void add_path(const std::string& path);

    /// \brief Return Http method name
    /// \return
    method get_method() const;

    /// \brief Return http method name as string
    /// \return string method name, ie GET, POST, etc
    std::string get_method_str() const;

    /// \brief Whether call will be with requested with ssl stream or not
    /// \return true if ssl is used
    bool is_ssl() const;

    /// \brief parse query string to vector<KeyValue>. String must not contains hostname or protocol, only query string.
    /// Example: ?id=1&param=2&someKey=3
    /// Warning! Keys represented as arrays, will not be recognized as arrays, they will stored as multiple values, of one keys,
    /// and if you will try to get param only by key using getParam(const std::string&), method will return only first found value, not all.
    /// \param query_string
    void parse_query(const std::string& query_string);

    /// \brief Check for at least one query parameter has set
    /// \return
    bool has_query() const;

    /// \brief Add query param key-value wss::web::KeyValue. Key can be array! Just set std::pair<std::string,std::string>("arr[]", "v0")
    /// \param keyValue pair of strings
    void add_query(kv&& keyValue);

    /// \brief Add query param key-value (value double type)
    /// \param keyValue pair of string=>uint64_t
    void add_query(kvd&& keyValue);

    /// \brief Add query param key-value (value double type)
    /// \param keyValue pair of string=>uint64_t
    void add_query(kvf&& keyValue);

    /// \brief Check for parameter exists
    /// \param key query parameter name
    /// \param icase search case sensititvity
    /// \return true if parameter exists
    bool has_query(const std::string& key, bool icase = true) const;

    /// \brief Return value of query parameter
    /// \param key query parameter name
    /// \param icase search case sensititvity
    /// \return empty string of parameter did not set
    std::string get_query_value(const std::string& key, bool icase = true) const;

    /// \brief
    /// \param key
    /// \param icase
    /// \return
    boost::optional<httb::kv> find_query(const std::string& key, bool icase = true) const;

    /// \brief Set query param with value
    /// \param key query parameter name
    /// \param value parameter value
    void set_query(const std::string& key, const std::string& value, bool icase = true);

    /// \brief Set query param by key-value pair
    /// \param kv key-value pair
    /// \param icase search case sensititvity
    void set_query(const httb::kv& kv, bool icase = true);

    /// \brief Remove query array paramter by it name and index
    /// Example: to delete first item of: a[]=1&a[]=2&a[]=3
    /// Use: remove_query_array_item("a[]", 0)
    /// OR
    /// remove_query_array_item("A[]", 0, true) to ignore input string case
    /// \param key param name ( !!! with braces: [] )
    /// \param index array index
    /// \param icase search case sensititvity
    /// \return true if was removed, false otherwise
    bool remove_query_array(const std::string& key, size_t index = 0, bool icase = true);

    /// \brief Remove query paramter by it name
    /// \param key param name
    /// \param icase use case insensitive search
    /// \return true if was removed, false otherwise
    bool remove_query(const std::string& key, bool icase = true);

    /// \brief Remove all existent queries
    void clear_queries();

    /// \brief Get size of total queries
    /// \return
    size_t queries_size() const;

    /// \brief Return multiple entries if param is an array.
    /// For example k[]=k&m[]=k&m=3, will return:
    /// std::vector<std::string> out(3);
    /// \param key
    /// \param icase
    /// \return vector of string pairs
    std::vector<std::string> get_query_array(const std::string& key, bool icase = true) const;

    /// \brief Build passed url with query parameters
    /// \return url with parameters. if url did not set, will return empty string without parameters
    std::string get_query_string() const;

    /// \brief Return vector of passed parameters
    /// \return simple vector with pairs of strings
    const kv_vector& get_query_list() const;

    // Use with carefully
    void parse_url(const std::string& url);

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

class HTTB_API request : public httb::base_request {
public:
    request()
        : base_request() {
    }
    explicit request(const std::string& url)
        : base_request(url) {
    }
    explicit request(const std::string& url, uint16_t port)
        : base_request(url, port) {
    }
    request(const std::string& url, base_request::method method)
        : base_request(url, method) {
    }

    [[nodiscard]] boost::beast::http::request<boost::beast::http::string_body> to_beast_request() const;

    void set_body(const std::string& body) override;
    void set_body(std::string&& body) override;
    void set_body(const httb::request_body& body);
    void set_body(httb::request_body&& body);
};

} // namespace httb

#endif //HTTB_REQUEST_H
