/*!
 * httb.
 * multipart.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "httb/body_multipart.h"
#include <random>

httb::multipart_entry::multipart_entry(const std::string &name, const std::string &body) :
    name(name),
    body(body) {
}

httb::multipart_entry::multipart_entry(
    const std::string &name,
    const std::string &contentType,
    const std::string &filename,
    const std::string &body) :
    name(name),
    body(body),
    contentType(contentType),
    filename(filename) {

}

httb::multipart_entry::multipart_entry(
    std::string &&name,
    std::string &&contentType,
    std::string &&filename,
    std::string &&body) :
    name(std::move(name)),
    body(std::move(body)),
    contentType(std::move(contentType)),
    filename(std::move(filename)) {

}

httb::body_multipart::body_multipart() {
    const std::string randomVal = generateRandomString(8);
    std::string boundaryName = "----HttbBoundary" + randomVal;
    this->boundaryName = std::move(boundaryName);
}

httb::body_multipart::~body_multipart() {

}

httb::body_multipart &httb::body_multipart::addEntry(httb::multipart_entry &&entry) {
    m_entries.push_back(std::move(entry));
    return *this;
}

#include <iostream>

const std::string httb::body_multipart::build(httb::io_container *request) const {
    std::stringstream ss;
    int i = 0;
    for (const auto &entry: m_entries) {
        if (entry.body.empty()) {
            // warn
            continue;
        }

        ss << "--" << boundaryName << "\r\n";
        ss << "Content-Disposition: format-data; name=\"" << entry.name << "\"";
        if (!entry.filename.empty()) {
            ss << "; filename=\"" << entry.filename << "\"";
            ss << "\r\n";
        } else {
            ss << "\r\n";
        }

        if (!entry.contentType.empty()) {
            ss << "Content-Type: " << entry.contentType << "\r\n";
        }

        ss << "\r\n";
        ss << entry.body;

        if(i < m_entries.size()-1) {
            ss << "\r\n";
        }
        i++;
    }

    request->setHeader({"Content-Type", "multipart/form-data; boundary=" + boundaryName});
    const std::string res = ss.str();

//    if(res.size() >= 1024) {
//        std::cout << res.substr(0, 1023) << "... (too big to print)" << std::endl;
//    } else {
//        std::cout << res << std::endl;
//    }


    return res;
}

const std::string httb::body_multipart::generateRandomString(int length) {
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



 