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

#include "types.h"

#include <boost/optional.hpp>
#include <string>
#include <utility>

namespace httb {

class io_container {
public:
    io_container();

    /// \brief Set request body data
    /// \param data string data for request/response
    virtual void set_body(const std::string& data);

    /// \brief Move request body data
    /// \param data string data for request/response
    virtual void set_body(std::string&& data);

    /// \brief Set header. Overwrites if already contains
    /// \param key_value std::pair<std::string, std::string>
    void set_header(httb::kv&& key_value);

    /// \brief Adds from map new headers values, if some key exists, value will overwrited
    /// \see add_header(const KeyValue&)
    /// \param map unorderd_map
    void set_headers(const httb::icase_map_t& map);

    /// \brief Adds from map new headers values, if some key exists, value will overwrited
    /// \see add_header(const KeyValue&)
    /// \param mmp
    void set_headers(const httb::icase_multimap_t& mmp);

    /// \brief Check for header keys exists
    /// \param name header name. Searching is case insensitive
    /// \return true is key exists
    bool has_header(const std::string& name) const;

    /// \brief Search for header and return row as pair: wss::web::KeyValue
    /// \param name string. Searching is case insensitive
    /// \return pair wss::web::KeyValue
    boost::optional<httb::kv> find_header_pair(const std::string& name) const;

    /// \brief Search for header and return it value
    /// \param headerName string. Searching is case insensitive
    /// \return empty string if not found, otherwise copy of origin value
    std::string get_header_value(const std::string& headerName) const;

    /// \brief Search for header and compare it value with comparable string
    /// \param header_name string. Searching is case insensitive
    /// \param comparable string to compare with
    /// \return true if header found and equals to comparable, false otherwise
    bool cmp_header_value(const std::string& header_name, const std::string& comparable) const;

    /// \brief Add header with separate key and value strings. If header exists, value will be ovewrited.
    /// \param key string. Value will be writed in original case
    /// \param value any string
    void add_header(const std::string& key, const std::string& value);

    /// \brief Add header with pair of key and value. If header exists, value will be ovewrited by new value.
    /// \see wss::web::KeyValue
    /// \param kv
    void add_header(const httb::kv& kv);

    /// \brief Move input pair wss::web::KeyValue to header map. If header exists, value will be ovewrited.
    /// \param kv
    /// \see wss::web::KeyValue
    void add_header(kv&& kv);

    /// \brief Add headers collection of key and value. If some header exists, value wil be overwrited by new value
    /// \@see KeyValueVector
    /// \param values
    void add_headers(const kv_vector& values);

    /// \brief Remove header by it's name
    /// \param name header name in any case you want
    /// \param icase true - case insensitive search
    /// \return true if was removed, false - if not
    bool remove_header(const std::string& name, bool icase = true);

    /// \brief Remove all existent headers
    void clear_headers();

    /// \brief Get size of headers
    /// \return
    size_t headers_size() const;

    /// \brief Get copy of request/response body
    /// \return Copy of body
    virtual std::string get_body() const;

    /// \brief Get copy of request/response body in char*
    /// \return Copy of body
    virtual const char* get_body_c() const;

    /// \brief Check for body is not empty
    /// \return true if !body.empty()
    virtual bool has_body() const;

    /// \brief
    /// \return Return body length in bytes
    virtual std::size_t get_body_size() const;

    virtual void clear_body();

    /// \brief Check for header map has at least one value
    /// \return true if map not empty
    bool has_headers() const;

    /// \brief Return copy of header map
    /// \see wss::web::KeyValueVector
    /// \see wss::web::keyValue
    /// \return simple vector of pairs std::vector<KeyValue>
    const httb::kv_vector& get_headers() const;

    /// \brief Glue headers and return list of its.
    /// \return vector of strings:
    /// Example:
    /// {
    ///     "Content-Type: application/json",
    ///     "Connection: keep-alive"
    /// }
    std::vector<std::string> get_headers_glued() const;

protected:
    httb::kv_vector m_headers;
    std::string m_body;
};

} // namespace httb

#endif //HTTB_IO_CONTAINER_H
