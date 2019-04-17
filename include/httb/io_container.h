/*!
 * httb.
 * io_container.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_IO_CONTAINER_H
#define HTTB_IO_CONTAINER_H

#include <string>
#include <boost/utility/string_view.hpp>
#include "defs.h"

namespace httb {
class io_container {
public:
    io_container();

    virtual /// \brief Set request body data
    /// \param data string data for request/response
    void setBody(const std::string &data);

    virtual /// \brief Move request body data
    /// \param data string data for request/response
    void setBody(std::string &&data);

    /// \brief Set header. Overwrites if already contains
    /// \param keyValue std::pair<std::string, std::string>
    void setHeader(httb::kv &&keyValue);

    /// \brief Adds from map new headers values, if some key exists, value will overwrited
    /// \see addHeader(const KeyValue&)
    /// \param map unorderd_map
    void setHeaders(const httb::CaseInsensitiveMap &map);

    /// \brief Adds from map new headers values, if some key exists, value will overwrited
    /// \see addHeader(const KeyValue&)
    /// \param mmp
    void setHeaders(const httb::CaseInsensitiveMultimap &mmp);

    /// \brief Check for header keys exists
    /// \param name header name. Searching is case insensitive
    /// \return true is key exists
    bool hasHeader(const std::string &name) const;

    /// \brief Search for header and return row as pair: wss::web::KeyValue
    /// \param headerName string. Searching is case insensitive
    /// \return pair wss::web::KeyValue
    std::pair<std::string, std::string> getHeaderPair(const std::string &headerName) const;

    /// \brief Search for header and return it value
    /// \param headerName string. Searching is case insensitive
    /// \return empty string if not found, otherwise copy of origin value
    std::string getHeader(const std::string &headerName) const;

    /// \brief Search for header and compare it value with comparable string
    /// \param headerName string. Searching is case insensitive
    /// \param comparable string to compare with
    /// \return true if header found and equals to comparable, false otherwise
    bool compareHeaderValue(const std::string &headerName, const std::string &comparable) const;

    /// \brief Add header with separate key and value strings. If header exists, value will be ovewrited.
    /// \param key string. Value will be writed in original case
    /// \param value any string
    void addHeader(const boost::string_view &key, const boost::string_view &value);

    /// \brief Add header with separate key and value strings. If header exists, value will be ovewrited.
    /// \param key string. Value will be writed in original case
    /// \param value any string
    void addHeader(const std::string &key, const std::string &value);

    /// \brief Add header with pair of key and value. If header exists, value will be ovewrited by new value.
    /// \see wss::web::KeyValue
    /// \param keyValue
    void addHeader(const httb::kv &keyValue);

    /// \brief Move input pair wss::web::KeyValue to header map. If header exists, value will be ovewrited.
    /// \param keyValue
    /// \see wss::web::KeyValue
    void addHeader(kv &&keyValue);

    /// \brief Add headers collection of key and value. If some header exists, value wil be overwrited by new value
    /// \@see KeyValueVector
    /// \param values
    void addHeaders(const kv_vector &values);

    /// \brief Get copy of request/response body
    /// \return Copy of body
    virtual std::string getBody() const;

    /// \brief Get copy of request/response body in char*
    /// \return Copy of body
    virtual const char *getBodyC() const;

    /// \brief Check for body is not empty
    /// \return true if !body.empty()
    virtual bool hasBody() const;

    /// \brief
    /// \return Return body length in bytes
    virtual std::size_t getBodySize() const;

    /// \brief Check for header map has at least one value
    /// \return true if map not empty
    bool hasHeaders() const;

    /// \brief Return copy of header map
    /// \see wss::web::KeyValueVector
    /// \see wss::web::keyValue
    /// \return simple vector of pairs std::vector<KeyValue>
    const httb::kv_vector &getHeaders() const;

    /// \brief Glue headers and return list of its.
    /// \return vector of strings:
    /// Example:
    /// {
    ///     "Content-Type: application/json",
    ///     "Connection: keep-alive"
    /// }
    std::vector<std::string> getHeadersGlued() const;

protected:
    httb::kv_vector headers;
    std::string body;
};
}

#endif //HTTB_IO_CONTAINER_H
