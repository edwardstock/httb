/*!
 * wsserver.
 * Response.cpp
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include <toolboxpp.hpp>
#include "httb/defs.h"
#include "httb/response.h"

httb::kv_vector httb::response::parseFormUrlEncode() const {
    if (data.empty()) {
        return {};
    }

    std::vector<std::string> groups = toolboxpp::strings::split(data, '&');

    kv_vector kvData;
    for (auto &&s: groups) {
        kvData.push_back(toolboxpp::strings::splitPair(s, "="));
    }

    return kvData;
}
void httb::response::dump() const {
    std::cout << "Response: " << std::endl
              << "  Status: " << status << std::endl
              << " Message: " << statusMessage << std::endl
              << "    Body: " << data << std::endl
              << " Headers:\n";
    for (const auto &h: headers) {
        std::cout << "\t" << h.first << ": " << h.second << std::endl;
    }
}
bool httb::response::isSuccess() const {
    return statusCode >= 200 && statusCode < 400;
}
std::string httb::response::getBody() const {
    return data;
}
const char *httb::response::getBodyC() const {
    return data.c_str();
}
bool httb::response::hasBody() const {
    return !data.empty();
}
void httb::response::setBody(const std::string &body) {
    data = body;
}
void httb::response::setBody(std::string &&body) {
    data = std::move(body);
}
size_t httb::response::getBodySize() const {
    return data.length();
}
bool httb::response::isInternalError() {
    return statusCode >= INTERNAL_ERROR_OFFSET || statusCode < 0;
}
