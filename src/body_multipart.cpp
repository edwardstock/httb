/*!
 * httb.
 * multipart.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "httb/body_multipart.h"

#include "utils.h"

#include <iostream>
#include <random>
#include <toolbox/io.h>

httb::multipart_entry::multipart_entry(const std::string& name, const std::string& body)
    : m_name(name),
      m_body(body) {
}

httb::multipart_entry::multipart_entry(const std::string& name, const httb::file_path_entry& pathEntry)
    : m_name(name),
      m_contentType(pathEntry.content_type),
      m_filename(pathEntry.filename) {
    m_bodyLoader = [pathEntry] {
        return toolbox::io::file_read_full(pathEntry.path);
    };
}
httb::multipart_entry::multipart_entry(const std::string& name, const httb::file_body_entry& bodyEntry)
    : m_name(name),
      m_contentType(bodyEntry.content_type),
      m_filename(bodyEntry.filename),
      m_body(bodyEntry.body) {
}

std::string httb::multipart_entry::body() const {
    if (m_body.empty()) {
        return m_bodyLoader();
    }

    return m_body;
}

std::string httb::multipart_entry::name() const {
    return m_name;
}
std::string httb::multipart_entry::content_type() const {
    return m_contentType;
}
std::string httb::multipart_entry::filename() const {
    return m_filename;
}

bool httb::multipart_entry::has_body() const {
    return !m_body.empty() || (m_bodyLoader && !m_filename.empty());
}

httb::body_multipart::body_multipart() {
    const std::string randomVal = httb::gen_random_string(8);
    std::string bname = "----HttbBoundary" + randomVal;
    this->boundaryName = std::move(bname);
}

httb::body_multipart::~body_multipart() {
}

httb::body_multipart& httb::body_multipart::add_entry(httb::multipart_entry&& entry) {
    m_entries.push_back(std::move(entry));
    return *this;
}

std::string httb::body_multipart::build(httb::io_container* request) const {
    std::stringstream ss;
    size_t i = 0;
    for (auto&& entry : m_entries) {
        if (!entry.has_body()) {
            // warn
            continue;
        }

        ss << "--" << boundaryName << "\r\n";
        ss << "Content-Disposition: format-data; name=\"" << entry.name() << "\"";
        if (!entry.filename().empty()) {
            ss << "; filename=\"" << entry.filename() << "\"";
            ss << "\r\n";
        } else {
            ss << "\r\n";
        }

        if (!entry.content_type().empty()) {
            ss << "Content-Type: " << entry.content_type() << "\r\n";
        }

        ss << "\r\n";
        ss << entry.body();

        if (i < m_entries.size() - 1) {
            ss << "\r\n";
        }
        i++;
    }

    request->set_header({"Content-Type", "multipart/form-data; boundary=" + boundaryName});
    const std::string res = ss.str();

    return res;
}
