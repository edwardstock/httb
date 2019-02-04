/*!
 * wsserver.
 * Response.cpp
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include <toolboxpp.h>
#include "httb/defs.h"
#include "httb/response.h"

//nlohmann::json httb::response::parseJsonBody() const {
//    if (data.empty()) {
//        return nlohmann::json();
//    }
//
//    nlohmann::json out;
//    try {
//        out = nlohmann::json::parse(data);
//    } catch (const std::exception &e) {
//        std::cerr << "Can't parse incoming json data: " << e.what() << std::endl;
//    }
//
//    return out;
//}
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
    for (auto h: headers) {
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
