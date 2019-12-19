/*!
 * httb.
 * multipart.hpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_MULTIPART_HPP
#define HTTB_MULTIPART_HPP

#include "httb/body.h"
#include "httb/httb_config.h"

#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace httb {

struct file_body_entry {
    std::string filename;
    std::string content_type;
    std::string body;
};

struct file_path_entry {
    std::string filename;
    std::string content_type;
    std::string path;
};

class HTTB_API multipart_entry {
public:
    using body_resolver_func = std::function<std::string(void)>;
    /// \brief Simple key-value entry, like basic POST request with fields data
    /// \param name param name
    /// \param body param value
    multipart_entry(const std::string& name, const std::string& body);
    multipart_entry(const std::string& name, const file_path_entry& pathEntry);
    multipart_entry(const std::string& name, const file_body_entry& bodyEntry);

    std::string body() const;
    std::string name() const;
    std::string content_type() const;
    std::string filename() const;
    bool has_body() const;

private:
    body_resolver_func m_bodyLoader;
    std::string m_name;
    std::string m_contentType = "text/plain";
    std::string m_filename;
    std::string m_body;
};

class HTTB_API body_multipart : public httb::request_body {
public:
    std::string boundaryName;
    std::vector<httb::multipart_entry> m_entries;
    body_multipart();
    virtual ~body_multipart();

    body_multipart& add_entry(httb::multipart_entry&& entry);
    std::string build(httb::io_container* request) const override;
};
} // namespace httb

#endif //HTTB_MULTIPART_HPP
