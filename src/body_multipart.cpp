/*!
 * httb.
 * multipart.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include <random>
#include <iostream>
#include <toolboxpp.hpp>
#include "httb/body_multipart.h"

httb::multipart_entry::multipart_entry(const std::string &name, const std::string &body) :
    m_name(name),
    m_body(body) {
}

httb::multipart_entry::multipart_entry(const std::string &name, const httb::file_path_entry &pathEntry) :
    m_name(name),
    m_contentType(pathEntry.contentType),
    m_filename(pathEntry.filename) {
    m_bodyLoader = [pathEntry] {
      return toolboxpp::fs::readFile(pathEntry.path);
    };
}
httb::multipart_entry::multipart_entry(const std::string &name, const httb::file_body_entry &bodyEntry):
    m_name(name),
    m_contentType(bodyEntry.contentType),
    m_filename(bodyEntry.filename),
    m_body(bodyEntry.body) {

}

std::string httb::multipart_entry::getBody() const {
    if (m_body.empty()) {
        return m_bodyLoader();
    }

    return m_body;
}

std::string httb::multipart_entry::getName() const {
    return m_name;
}
std::string httb::multipart_entry::getContentType() const {
    return m_contentType;
}
std::string httb::multipart_entry::getFilename() const {
    return m_filename;
}

bool httb::multipart_entry::hasBody() const {
    return !m_body.empty() || (m_bodyLoader && !m_filename.empty());
}

httb::body_multipart::body_multipart() {
    const std::string randomVal = genRandomString(8);
    std::string bname = "----HttbBoundary" + randomVal;
    this->boundaryName = std::move(bname);
}

httb::body_multipart::~body_multipart() {

}

httb::body_multipart &httb::body_multipart::addEntry(httb::multipart_entry &&entry) {
    m_entries.push_back(std::move(entry));
    return *this;
}

std::string httb::body_multipart::build(httb::io_container *request) const {
    std::stringstream ss;
    size_t i = 0;
    for (auto &&entry: m_entries) {
        if (!entry.hasBody()) {
            // warn
            continue;
        }

        ss << "--" << boundaryName << "\r\n";
        ss << "Content-Disposition: format-data; name=\"" << entry.getName() << "\"";
        if (!entry.getFilename().empty()) {
            ss << "; filename=\"" << entry.getFilename() << "\"";
            ss << "\r\n";
        } else {
            ss << "\r\n";
        }

        if (!entry.getContentType().empty()) {
            ss << "Content-Type: " << entry.getContentType() << "\r\n";
        }

        ss << "\r\n";
        ss << entry.getBody();

        if (i < m_entries.size() - 1) {
            ss << "\r\n";
        }
        i++;
    }

    request->setHeader({"Content-Type", "multipart/form-data; boundary=" + boundaryName});
    const std::string res = ss.str();

    return res;
}

std::string httb::body_multipart::genRandomString(int length) {
    std::string chars(
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "1234567890");

    std::random_device rng;
    std::uniform_int_distribution<> index_dist(0, static_cast<int>(chars.size() - 1));
    std::stringstream ss;
    for (int i = 0; i < length; ++i) {
        ss << chars[index_dist(rng)];
    }

    return ss.str();
}



 