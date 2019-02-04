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
class multipart_entry {
public:
    std::string name;
    std::string body;
    std::string contentType = "text/plain";
    std::string filename;

    multipart_entry(const std::string &name, const std::string &body);
    multipart_entry(const std::string &name, const std::string &contentType, const std::string &filename, const std::string &body);
    multipart_entry(std::string &&name, std::string &&contentType, std::string &&filename, std::string &&body);
};

class body_multipart: public httb::request_body {
public:
    std::string boundaryName;
    std::vector<httb::multipart_entry> m_entries;
    body_multipart();

    virtual ~body_multipart();

    body_multipart &addEntry(httb::multipart_entry &&entry);

    const std::string build(httb::io_container *request) const override;

private:
    static const std::string generateRandomString(int length);
};
}

#endif //HTTB_MULTIPART_HPP
