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

#include <string>
#include <sstream>
#include <vector>
#include "httb/body.h"

namespace httb {

struct file_body_entry {
  std::string filename;
  std::string contentType;
  std::string body;
};

struct file_path_entry {
  std::string filename;
  std::string contentType;
  std::string path;
};

class multipart_entry {
public:
    using body_resolver_func = std::function<std::string(void)>;
    /// \brief Simple key-value entry, like basic POST request with fields data
    /// \param name param name
    /// \param body param value
    multipart_entry(const std::string &name, const std::string &body);

    multipart_entry(const std::string &name, const file_path_entry &pathEntry);

    multipart_entry(const std::string &name, const file_body_entry &bodyEntry);

    std::string getBody() const;
    std::string getName() const;
    std::string getContentType() const;
    std::string getFilename() const;
    bool hasBody() const;

private:
    body_resolver_func m_bodyLoader;
    std::string m_name;
    std::string m_contentType = "text/plain";
    std::string m_filename;
    std::string m_body;
};

class body_multipart: public httb::request_body {
public:
    std::string boundaryName;
    std::vector<httb::multipart_entry> m_entries;
    body_multipart();

    virtual ~body_multipart();

    body_multipart &addEntry(httb::multipart_entry &&entry);

    std::string build(httb::io_container *request) const override;

private:
    static std::string genRandomString(int length);
};
}

#endif //HTTB_MULTIPART_HPP
