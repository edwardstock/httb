/*!
 * httb.
 * body_formurlencoded.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "httb/body_form_urlencoded.h"

httb::body_form_urlencoded &httb::body_form_urlencoded::addParam(const httb::kv &param) {
    params.push_back(param);
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::addParam(httb::kv &&param) {
    params.push_back(std::move(param));
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::addParams(const httb::kv_vector &append) {
    params.insert(params.end(), append.begin(), append.end());
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::addParams(httb::kv_vector &&append) {
    for (auto &&item: append) {
        params.push_back(std::move(item));
    }
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::addParam(const std::string &name,
                                                                 const std::vector<std::string> &values) {
    for(const auto& val: values) {
        addParam({name+"[]", val});
    }
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::addParams(const std::unordered_map<std::string,
                                                                                           std::string> &map) {
    for(const auto& item: map) {
        addParam(item);
    }
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::addParams(const std::unordered_multimap<std::string,
                                                                                                std::string> &map) {
    for(const auto& item: map) {
        addParam(item);
    }
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::addParams(const std::map<std::string, std::string> &map) {
    for(const auto& item: map) {
        addParam(item);
    }
    return *this;
}
httb::body_form_urlencoded &httb::body_form_urlencoded::addParams(const std::multimap<std::string, std::string> &map) {
    for(const auto& item: map) {
        addParam(item);
    }
    return *this;
}
const std::string httb::body_form_urlencoded::build(httb::io_container *request) const {
    std::stringstream out;
    int i = 0;
    for (auto &h: params) {
        out << h.first << "=" << h.second;
        if (i != params.size() - 1) {
            out << "&";
        }
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

}
void httb::body_form_urlencoded::parseParams(std::string &&encoded) {

}
