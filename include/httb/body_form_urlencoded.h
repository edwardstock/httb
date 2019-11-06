/*!
 * httb.
 * body_formurlencoded.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_BODY_FORMURLENCODED_H
#define HTTB_BODY_FORMURLENCODED_H

#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include "httb/defs.h"
#include "httb/body.h"

namespace httb {

class body_form_urlencoded: public httb::request_body {
public:
    body_form_urlencoded() = default;
    body_form_urlencoded(const std::string &encodedParamsString);
    body_form_urlencoded(std::string &&encodedParamsString);

    body_form_urlencoded& add_param(const httb::kv &param);
    body_form_urlencoded& add_param(httb::kv&& param);
    body_form_urlencoded& add_param(const std::string &name, const std::vector<std::string> &values);
    body_form_urlencoded& add_params(const httb::kv_vector& append);
    body_form_urlencoded& add_params(httb::kv_vector &&append);
    body_form_urlencoded& add_params(const std::unordered_map<std::string, std::string> &map);
    body_form_urlencoded& add_params(const std::unordered_multimap<std::string, std::string> &map);
    body_form_urlencoded& add_params(const std::map<std::string, std::string> &map);
    body_form_urlencoded& add_params(const std::multimap<std::string, std::string> &map);

    std::string build(httb::io_container *request) const override;

    virtual ~body_form_urlencoded();

private:
    httb::kv_vector params;

    void parseParams(const std::string &encoded);
    void parseParams(std::string&& encoded);
};

}

#endif //HTTB_BODY_FORMURLENCODED_H
