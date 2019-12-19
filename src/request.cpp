/*!
 * wsserver.
 * Request.cpp
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "httb/request.h"

#include "utils.h"

#include <boost/regex.hpp>
#include <httb/types.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <toolbox/strings.hpp>
#include <toolbox/strings/regex.h>

// REQUEST
httb::base_request::base_request()
    : io_container(),
      m_ssl(false),
      m_method(method::get),
      m_proto("http"),
      m_host(""),
      m_port("80"),
      m_path("/") {
}

httb::base_request::base_request(const std::string& url)
    : io_container(),
      m_ssl(false),
      m_method(method::get),
      m_proto("http"),
      m_host(""),
      m_port("80"),
      m_path("/") {
    parse_url(url);
}

httb::base_request::base_request(const std::string& url, uint16_t port)
    : io_container(),
      m_ssl(false),
      m_method(method::get),
      m_proto("http"),
      m_host(""),
      m_path("/") {
    std::stringstream ss;
    ss << port;
    m_port = ss.str();
    parse_url(url);
}

httb::base_request::base_request(const std::string& url, httb::base_request::method method)
    : io_container(),
      m_ssl(false),
      m_method(method),
      m_proto("http"),
      m_host(""),
      m_port("80"),
      m_path("/") {
    parse_url(url);
}

inline std::vector<std::string> mreg(const boost::regex& re, const std::string& source) {
    boost::smatch result;
    boost::regex_search(source, result, re);

    std::vector<std::string> out(result.size());
    const size_t cnt = result.size();
    for (size_t i = 0; i < cnt; i++) {
        out[i] = result[i];
    }

    return out;
}

void httb::base_request::parse_url(const std::string& url) {
    std::string urlParseRegex =
        R"(([a-zA-Z]+)\:\/\/([a-zA-Z0-9\.\-_]+)?\:?([0-9]{1,5})?(\/[a-zA-Z0-9\/\+\-\.\%\/_]*)?\??([a-zA-Z0-9\-_\+\=\&\%\.]*))";

    auto res = mreg(boost::regex(urlParseRegex), url);
    if (!toolbox::strings::matches_pattern(urlParseRegex, url)) {
        return;
    }

    m_proto = res[1];
    m_host = res[2];
    std::string port = res[3];
    m_path = res[4];
    std::string paramsString = res[5];

    if (toolbox::strings::equals_icase(m_proto, "https")) {
        m_port = "443";
        m_ssl = true;
    } else if (toolbox::strings::equals_icase(m_proto, "ftp")) {
        m_port = "20";
        m_ssl = false;
    }

    if (!port.empty()) {
        m_port = port;
    }

    if (!paramsString.empty()) {
        auto kvs = toolbox::strings::split(paramsString, '&');
        for (auto& item : kvs) {
            auto kv = toolbox::strings::split_pair(item, '=');
            m_params.push_back({kv.first, kv.second});
        }
    }
}

void httb::base_request::parse_query(const std::string& query_string) {
    std::string query = query_string;
    if (query[0] == '?') {
        query = query.substr(1, query.length());
    }

    std::vector<std::string> pairs = toolbox::strings::split(query, "&");

    for (const auto& param : pairs) {
        add_query(toolbox::strings::split_pair(param, "="));
    }
}

httb::base_request::method httb::base_request::method_from_string(const std::string& method_name) {
    using toolbox::strings::equals_icase;

    if (equals_icase(method_name, "POST")) {
        return httb::base_request::method::post;
    } else if (equals_icase(method_name, "PUT")) {
        return method::put;
    } else if (equals_icase(method_name, "DELETE")) {
        return method::delete_;
    } else if (equals_icase(method_name, "HEAD")) {
        return method::head;
    }

    return method::get;
}

std::string httb::base_request::method_to_string(httb::base_request::method methodName) {
    std::string out;

    switch (methodName) {
    case method::post:
        out = "POST";
        break;
    case method::put:
        out = "PUT";
        break;
    case method::delete_:
        out = "DELETE";
        break;
    case method::head:
        out = "HEAD";
        break;
    case method::get:
        out = "GET";
        break;

    default:
        out = "UnsupportedMethod";
    }

    return out;
}

void httb::base_request::set_method(httb::base_request::method method) {
    this->m_method = method;
}

void httb::base_request::add_query(httb::kv&& keyValue) {
    m_params.push_back(std::move(keyValue));
}

void httb::base_request::add_query(kvd&& keyValue) {
    auto tmp = std::move(keyValue);
    std::stringstream ss;
    ss << tmp.second;

    add_query({tmp.first, ss.str()});
}

void httb::base_request::add_query(httb::kvf&& keyValue) {
    auto tmp = std::move(keyValue);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(7) << tmp.second;

    add_query({tmp.first, ss.str()});
}

void httb::base_request::use_ssl(bool use) {
    m_ssl = use;
}

std::string httb::base_request::get_url() const {
    std::stringstream ss;

    ss << m_proto << "://";
    ss << m_host;
    if (m_port != "80" && m_port != "443") {
        ss << ":" << m_port;
    }

    if (!m_path.empty()) {
        ss << m_path;
    } else {
        ss << "/";
    }

    if (has_query()) {
        ss << get_query_string();
    }

    return ss.str();
}

httb::base_request::method httb::base_request::get_method() const {
    return m_method;
}

std::string httb::base_request::get_method_str() const {
    return method_to_string(get_method());
}

bool httb::base_request::has_query() const {
    return !m_params.empty();
}

bool httb::base_request::has_query(const std::string& key, bool icase) const {
    using toolbox::strings::equals_icase;
    const auto& cmp = [icase](const std::string& lhs, const std::string& rhs) {
        if (icase) {
            return equals_icase(lhs, rhs);
        } else {
            return lhs == rhs;
        }
    };

    for (const auto& param : m_params) {
        if (cmp(param.first, key)) {
            return true;
        }
    }

    return false;
}

std::string httb::base_request::get_query_value(const std::string& key, bool icase) const {
    using toolbox::strings::equals_icase;
    const auto& cmp = [icase](const std::string& lhs, const std::string& rhs) {
        if (icase) {
            return equals_icase(lhs, rhs);
        } else {
            return lhs == rhs;
        }
    };
    for (const auto& param : m_params) {
        if (cmp(param.first, key)) {
            return param.second;
        }
    }

    return std::string();
}

boost::optional<httb::kv> httb::base_request::find_query(const std::string& key, bool icase) const {
    boost::optional<httb::kv> out;
    using toolbox::strings::equals_icase;
    const auto& cmp = [icase](const std::string& lhs, const std::string& rhs) {
        if (icase) {
            return equals_icase(lhs, rhs);
        } else {
            return lhs == rhs;
        }
    };
    for (const auto& param : m_params) {
        if (cmp(param.first, key)) {
            out = param;
            break;
        }
    }

    return out;
}

void httb::base_request::set_query(const std::string& key, const std::string& value, bool icase) {
    using toolbox::strings::equals_icase;
    const auto& cmp = [icase](const std::string& lhs, const std::string& rhs) {
        if (icase) {
            return equals_icase(lhs, rhs);
        } else {
            return lhs == rhs;
        }
    };
    for (auto& param : m_params) {
        if (cmp(param.first, key)) {
            param.second = value;
        }
    }
}

void httb::base_request::set_query(const httb::kv& kv, bool icase) {
    set_query(kv.first, kv.second, icase);
}

bool httb::base_request::remove_query_array(const std::string& key, size_t index, bool icase) {
    using toolbox::strings::equals_icase;
    const auto& cmp = [icase](const std::string& lhs, const std::string& rhs) {
        if (icase) {
            return equals_icase(lhs, rhs);
        } else {
            return lhs == rhs;
        }
    };

    size_t foundCount = 0;
    size_t erasedItems = 0;

    auto it = m_params.begin();
    while (it != m_params.end()) {
        if (cmp(it->first, key)) {
            if (foundCount == index) {
                it = m_params.erase(it);
                erasedItems++;
            } else {
                ++it;
            }
            foundCount++;
        } else {
            ++it;
        }
    }

    return erasedItems > 0;
}

bool httb::base_request::remove_query(const std::string& key, bool icase) {
    using toolbox::strings::equals_icase;
    const auto& cmp = [icase](const std::string& lhs, const std::string& rhs) {
        if (icase) {
            return equals_icase(lhs, rhs);
        } else {
            return lhs == rhs;
        }
    };

    int foundCount = 0;

    auto it = m_params.begin();
    while (it != m_params.end()) {
        if (cmp(it->first, key)) {
            it = m_params.erase(it);
            foundCount++;
        } else {
            ++it;
        }
    }

    return foundCount > 0;
}

void httb::base_request::clear_queries() {
    m_params.clear();
}

size_t httb::base_request::queries_size() const {
    return m_params.size();
}

std::vector<std::string> httb::base_request::get_query_array(const std::string& key, bool icase) const {
    std::vector<std::string> out;

    using toolbox::strings::equals_icase;
    const auto& cmp = [icase](const std::string& lhs, const std::string& rhs) {
        if (icase) {
            return equals_icase(lhs, rhs);
        } else {
            return lhs == rhs;
        }
    };

    for (const auto& param : m_params) {
        if (cmp(param.first, key)) {
            out.push_back(param.second);
        }
    }

    return out;
}

std::string httb::base_request::get_query_string() const {
    std::string combined;
    if (!m_params.empty()) {
        std::stringstream ss;
        std::vector<std::string> enc;
        for (auto& p : m_params) {
            ss << p.first << "=" << p.second;
            enc.push_back(ss.str());
            ss.str("");
            ss.clear();
        }
        std::string glued = toolbox::strings::glue("&", enc);
        combined += "?" + glued;
    }

    return combined;
}

const httb::kv_vector& httb::base_request::get_query_list() const {
    return m_params;
}

std::string httb::base_request::get_host() const {
    return m_host;
}

void httb::base_request::set_host(const std::string& hostname) {
    m_host = hostname;
}

uint16_t httb::base_request::get_port() const {
    const int port = std::stoi(m_port);
    return (uint16_t) port;
}

void httb::base_request::set_port(uint16_t portNumber) {
    std::stringstream ss;
    ss << portNumber;
    m_port = ss.str();
}

std::string httb::base_request::get_port_str() const {
    return m_port;
}

std::string httb::base_request::get_proto_name() const {
    return m_proto;
}

void httb::base_request::set_proto_name(const std::string& protocolName) {
    m_proto = protocolName;
}

std::string httb::base_request::get_path() const {
    if (m_path.empty()) {
        return "/";
    }
    return m_path;
}

std::string httb::base_request::get_path_with_query() const {
    return get_path() + get_query_string();
}

void httb::base_request::set_path(const std::string& path) {
    if (m_path.length() == 0) {
        m_path = "/";
    } else if (m_path.length() > 0 && m_path[0] != '/') {
        m_path = "/" + m_path;
    }

    if (path.length() > 1 && path[0] != '/') {
        m_path = "/" + path;
    } else {
        m_path = path;
    }
}

void httb::base_request::add_path(const std::string& path) {
    if (path.length() == 0 || (path.length() == 1 && path[0] == '/')) {
        return;
    }

    if (m_path.length() == 0) {
        m_path = "/";
    }

    if (path[0] == '/') {
        if (m_path[m_path.length() - 1] == '/') {
            m_path += path.substr(1, path.length());
        } else {
            m_path += path;
        }
    } else {
        if (m_path[m_path.length() - 1] != '/') {
            m_path += '/' + path;
        } else {
            m_path += path;
        }
    }
}

bool httb::base_request::is_ssl() const {
    return m_ssl;
}

boost::beast::http::request<boost::beast::http::string_body> httb::request::to_beast_request() const {
    namespace http = boost::beast::http;

    http::request<http::string_body> req{get_method(), get_path_with_query(), 11};
    req.set(http::field::host, get_host());

    std::stringstream versionBuilder;
    versionBuilder << "httb/" << HTTB_VERSION;
    versionBuilder << " (boost " << BOOST_VERSION << ")";
    req.set(http::field::user_agent, versionBuilder.str());
    req.set(http::field::accept, "*/*");
    req.set(http::field::content_length, "0");

    for (const auto& field : get_headers()) {
        req.set(field.first, field.second);
    }

    if (has_body()) {
        req.body() = get_body();
        req.prepare_payload();
    }

    return req;
}
void httb::request::set_body(const std::string& body) {
    io_container::set_body(body);
}
void httb::request::set_body(std::string&& body) {
    io_container::set_body(body);
}
void httb::request::set_body(const httb::request_body& body) {
    io_container::set_body(body.build(this));
}
void httb::request::set_body(httb::request_body&& body) {
    std::string builtBody = body.build(this);
    io_container::set_body(std::move(builtBody));
}
