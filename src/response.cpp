/*!
 * wsserver.
 * Response.cpp
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "httb/response.h"

#include "httb/types.h"

#include <iostream>
#include <toolbox/strings.hpp>

httb::response::response()
    : code(200), status(http_status::ok), status_message("Ok") {
}

httb::kv_vector httb::response::parse_form_url_encode() const {
    if (data.empty()) {
        return {};
    }

    std::vector<std::string> groups = toolbox::strings::split(data, '&');

    kv_vector kvData;
    for (auto&& s : groups) {
        kvData.push_back(toolbox::strings::split_pair(s, "="));
    }

    return kvData;
}
void httb::response::dump() const {
    std::cout << "Response: " << std::endl
              << "  Status: " << status << std::endl
              << " Message: " << status_message << std::endl
              << "    Body: " << data << std::endl
              << " Headers:\n";
    for (const auto& h : m_headers) {
        std::cout << "\t" << h.first << ": " << h.second << std::endl;
    }
}
bool httb::response::success() const {
    return code >= 200 && code < 400;
}
std::string httb::response::get_body() const {
    return data;
}
std::string httb::response::get_body(bool clear) {
    if (!clear) {
        return data;
    }

    std::string b = std::move(data);
    return b;
}
const char* httb::response::get_body_c() const {
    return data.c_str();
}
bool httb::response::has_body() const {
    return !data.empty();
}
void httb::response::set_body(const std::string& body) {
    data = body;
}
void httb::response::set_body(std::string&& body) {
    data = std::move(body);
}
size_t httb::response::get_body_size() const {
    return data.length();
}
bool httb::response::is_internal_error() {
    return code >= INTERNAL_ERROR_OFFSET || code < 0;
}
httb::response::operator bool() const noexcept {
    return success();
}

std::ostream& operator<<(std::ostream& os, const httb::response& resp) {
    os << resp.get_body();
    return os;
}
