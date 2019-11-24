/*!
 * httb.
 * body_formurlencoded.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "httb/body_form_urlencoded.h"
#include <toolboxpp.hpp>

httb::body_form_urlencoded &httb::body_form_urlencoded::add_param(const httb::kv &param) {
    params.push_back(param);
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::add_param(httb::kv &&param) {
    params.push_back(std::move(param));
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::add_params(const httb::kv_vector &append) {
    params.insert(params.end(), append.begin(), append.end());
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::add_params(httb::kv_vector &&append) {
    for (auto &&item: append) {
        params.push_back(std::move(item));
    }
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::add_param(const std::string &name,
                                                                  const std::vector<std::string> &values) {
    for(const auto& val: values) {
        add_param({name + "[]", val});
    }
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::add_params(const std::unordered_map<std::string,
                                                                                            std::string> &map) {
    for(const auto& item: map) {
        add_param(item);
    }
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::add_params(const std::unordered_multimap<std::string,
                                                                                                 std::string> &map) {
    for(const auto& item: map) {
        add_param(item);
    }
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::add_params(const std::map<std::string, std::string> &map) {
    for(const auto& item: map) {
        add_param(item);
    }
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::add_params(const std::multimap<std::string, std::string> &map) {
    for(const auto& item: map) {
        add_param(item);
    }
    return *this;
}
std::string httb::body_form_urlencoded::build(httb::io_container *) const {
    std::stringstream out;
    size_t i = 0;
    for (auto &h: params) {
        out << h.first << "=" << h.second;
        if (i < params.size() - 1) {
            out << "&";
        }
        i++;
    }

    return out.str();
}
httb::body_form_urlencoded::~body_form_urlencoded() {

}

httb::body_form_urlencoded::body_form_urlencoded(const std::string &encodedParamsString) {
    parseParams(encodedParamsString);
}
httb::body_form_urlencoded::body_form_urlencoded(std::string &&encodedParamsString) {
    parseParams(std::move(encodedParamsString));
}
void httb::body_form_urlencoded::parseParams(const std::string &encoded) {
    std::string query = encoded;
    if (query[0] == '?') {
        query = query.substr(1, query.length());
    }

    std::vector<std::string> pairs = toolboxpp::strings::split(query, "&");

    for (const auto &param: pairs) {
        add_param(toolboxpp::strings::splitPair(param, "="));
    }
}
void httb::body_form_urlencoded::parseParams(std::string &&encoded) {
    std::string query = std::move(encoded);
    if (query[0] == '?') {
        query = query.substr(1, query.length());
    }

    std::vector<std::string> pairs = toolboxpp::strings::split(query, "&");

    for (const auto &param: pairs) {
        add_param(toolboxpp::strings::splitPair(param, "="));
    }
}
