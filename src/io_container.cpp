/*!
 * httb.
 * io_container.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include <toolboxpp.hpp>
#include <boost/utility/string_view.hpp>
#include "httb/io_container.h"
#include "helpers.hpp"


// BASE IO
httb::io_container::io_container() : body() { }

void httb::io_container::setBody(const std::string &data) {
    this->body = data;
    setHeader({"Content-Length", httb::utils::toString(this->body.length())});
}
void httb::io_container::setBody(std::string &&data) {
    this->body = std::move(data);
    setHeader({"Content-Length", httb::utils::toString(this->body.length())});
}
void httb::io_container::setHeader(httb::kv &&keyValue) {
    using toolboxpp::strings::equalsIgnoreCase;
    bool found = false;
    for (auto &kv: headers) {
        if (equalsIgnoreCase(kv.first, keyValue.first)) {
            kv.second = keyValue.second;
            found = true;
        }
    }

    if (!found) {
        return addHeader(std::move(keyValue));
    }
}
bool httb::io_container::hasHeader(const std::string &name) const {
    using toolboxpp::strings::equalsIgnoreCase;
    for (auto &h: headers) {
        if (equalsIgnoreCase(h.first, name)) {
            return true;
        }
    }

    return false;
}
std::pair<std::string, std::string> httb::io_container::getHeaderPair(const std::string &headerName) const {
    using toolboxpp::strings::equalsIgnoreCase;
    for (auto &h: headers) {
        if (equalsIgnoreCase(h.first, headerName)) {
            return {h.first, h.second};
        }
    }

    return {};
}
std::string httb::io_container::getHeader(const std::string &headerName) const {
    using toolboxpp::strings::equalsIgnoreCase;
    for (auto &h: headers) {
        if (equalsIgnoreCase(h.first, headerName)) {
            return h.second;
        }
    }

    return std::string();
}
bool httb::io_container::compareHeaderValue(const std::string &headerName, const std::string &comparable) const {
    if (!hasHeader(headerName)) return false;
    return getHeader(headerName) == comparable;
}
void httb::io_container::addHeader(const std::string &key, const std::string &value) {
    headers.emplace_back(key, value);
}
void httb::io_container::addHeader(const boost::string_view &key, const boost::string_view &value) {
    addHeader(key.to_string(), value.to_string());
}
void httb::io_container::addHeader(const httb::kv &keyValue) {
    headers.push_back(keyValue);
}
void httb::io_container::addHeader(httb::kv &&keyValue) {
    headers.push_back(std::move(keyValue));
}
void httb::io_container::addHeaders(const httb::kv_vector &values) {
    headers.insert(headers.end(), values.begin(), values.end());
}
void httb::io_container::setHeaders(const httb::CaseInsensitiveMap &map) {
    for (auto &h: map) {
        addHeader(h.first, h.second);
    }
}
void httb::io_container::setHeaders(const httb::CaseInsensitiveMultimap &mmp) {
    for (auto &h: mmp) {
        addHeader(h.first, h.second);
    }
}
std::string httb::io_container::getBody() const {
    return body;
}
const char *httb::io_container::getBodyC() const {
    const char *out = body.c_str();
    return out;
}
std::size_t httb::io_container::getBodySize() const {
    return body.length();
}

bool httb::io_container::hasBody() const {
    return !body.empty();
}
bool httb::io_container::hasHeaders() const {
    return !headers.empty();
}
httb::kv_vector httb::io_container::getHeaders() const {
    return headers;
}
std::vector<std::string> httb::io_container::getHeadersGlued() const {
    std::vector<std::string> out(headers.size());

    std::stringstream ss;
    int i = 0;
    for (const auto &h: headers) {
        ss << h.first << ": " << h.second;
        out[i] = ss.str();
        ss.str("");
        ss.clear();
        i++;
    }

    return out;
}
 