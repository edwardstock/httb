/*!
 * wsserver.
 * Request.cpp
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include <string>
#include <sstream>
#include <thread>
#include <boost/regex.hpp>
#include <toolboxpp.hpp>
#include <httb/types.h>
#include "httb/request.h"
#include "httb/helpers.hpp"

// REQUEST
httb::base_request::base_request() :
    io_container(),
    m_ssl(false),
    m_method(method::get),
    m_proto("http"),
    m_host(""),
    m_port("80"),
    m_path("/") {
}

httb::base_request::base_request(const std::string &url) :
    io_container(),
    m_ssl(false),
    m_method(method::get),
    m_proto("http"),
    m_host(""),
    m_port("80"),
    m_path("/") {
    parseUrl(url);
}

httb::base_request::base_request(const std::string &url, uint16_t port) :
    io_container(),
    m_ssl(false),
    m_method(method::get),
    m_proto("http"),
    m_host(""),
    m_path("/") {
    std::stringstream ss;
    ss << port;
    m_port = ss.str();
    parseUrl(url);
}

httb::base_request::base_request(const std::string &url, httb::base_request::method method) :
    io_container(),
    m_ssl(false),
    m_method(method),
    m_proto("http"),
    m_host(""),
    m_port("80"),
    m_path("/") {
    parseUrl(url);
}

inline std::vector<std::string> mreg(const boost::regex &re, const std::string &source) {
    boost::smatch result;
    boost::regex_search(source, result, re);

    std::vector<std::string> out(result.size());
    const size_t cnt = result.size();
    for (size_t i = 0; i < cnt; i++) {
        out[i] = result[i];
    }

    return out;
}

void httb::base_request::parseUrl(const std::string &url) {
    std::string urlParseRegex =
        R"(([a-zA-Z]+)\:\/\/([a-zA-Z0-9\.\-_]+)?\:?([0-9]{1,5})?(\/[a-zA-Z0-9\/\+\-\.\%\/_]*)?\??([a-zA-Z0-9\-_\+\=\&\%\.]*))";

    auto res = mreg(boost::regex(urlParseRegex), url);
    if (!toolboxpp::strings::hasRegex(urlParseRegex, url)) {
        return;
    }

    m_proto = res[1];
    m_host = res[2];
    std::string port = res[3];
    m_path = res[4];
    std::string paramsString = res[5];

    if (toolboxpp::strings::equalsIgnoreCase(m_proto, "https")) {
        m_port = "443";
        m_ssl = true;
    } else if (toolboxpp::strings::equalsIgnoreCase(m_proto, "ftp")) {
        m_port = "20";
        m_ssl = false;
    }

    if (!port.empty()) {
        m_port = port;
    }

    if (!paramsString.empty()) {
        auto kvs = toolboxpp::strings::split(paramsString, '&');
        for (auto &item: kvs) {
            auto kv = toolboxpp::strings::splitPair(item, '=');
            m_params.push_back({kv.first, kv.second});
        }
    }
}

void httb::base_request::parseQuery(const std::string &queryString) {
    std::string query = queryString;
    if (query[0] == '?') {
        query = query.substr(1, query.length());
    }

    std::vector<std::string> pairs = toolboxpp::strings::split(query, "&");

    for (const auto &param: pairs) {
        addQuery(toolboxpp::strings::splitPair(param, "="));
    }
}

httb::base_request::method httb::base_request::MethodFromString(const std::string &method_name) {
    using toolboxpp::strings::equalsIgnoreCase;

    if (equalsIgnoreCase(method_name, "POST")) {
        return httb::base_request::method::post;
    } else if (equalsIgnoreCase(method_name, "PUT")) {
        return method::put;
    } else if (equalsIgnoreCase(method_name, "DELETE")) {
        return method::delete_;
    } else if (equalsIgnoreCase(method_name, "HEAD")) {
        return method::head;
    }

    return method::get;
}

std::string httb::base_request::MethodToString(httb::base_request::method methodName) {
    std::string out;

    switch (methodName) {
        case method::post:out = "POST";
            break;
        case method::put:out = "PUT";
            break;
        case method::delete_:out = "DELETE";
            break;
        case method::head: out = "HEAD";
            break;
        case method::get: out = "GET";
            break;

        default: out = "UnsupportedMethod";
    }

    return out;
}

void httb::base_request::setMethod(httb::base_request::method method) {
    this->m_method = method;
}

void httb::base_request::addQuery(httb::kv &&keyValue) {
    m_params.push_back(std::move(keyValue));
}

void httb::base_request::addQuery(kvd &&keyValue) {
    auto tmp = std::move(keyValue);
    std::stringstream ss;
    ss << tmp.second;

    addQuery({tmp.first, ss.str()});
}

void httb::base_request::useSSL(bool useSSL) {
    m_ssl = useSSL;
}

std::string httb::base_request::getUrl() const {
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

    if (hasQuery()) {
        ss << getQueryString();
    }

    return ss.str();
}

httb::base_request::method httb::base_request::getMethod() const {
    return m_method;
}

bool httb::base_request::hasQuery() const {
    return !m_params.empty();
}

bool httb::base_request::hasQuery(const std::string &key, bool icase) const {
    using toolboxpp::strings::equalsIgnoreCase;
    const auto &cmp = [icase](const std::string &lhs, const std::string &rhs) {
      if (icase) {
          return equalsIgnoreCase(lhs, rhs);
      } else {
          return lhs == rhs;
      }
    };

    for (const auto &param: m_params) {
        if (cmp(param.first, key)) {
            return true;
        }
    }

    return false;
}

std::string httb::base_request::getQuery(const std::string &key, bool icase) const {
    using toolboxpp::strings::equalsIgnoreCase;
    const auto &cmp = [icase](const std::string &lhs, const std::string &rhs) {
      if (icase) {
          return equalsIgnoreCase(lhs, rhs);
      } else {
          return lhs == rhs;
      }
    };
    for (const auto &param: m_params) {
        if (cmp(param.first, key)) {
            return param.second;
        }
    }

    return std::string();
}

boost::optional<httb::kv> httb::base_request::getQueryEntry(const std::string &key, bool icase) const {
    boost::optional<httb::kv> out;
    using toolboxpp::strings::equalsIgnoreCase;
    const auto &cmp = [icase](const std::string &lhs, const std::string &rhs) {
      if (icase) {
          return equalsIgnoreCase(lhs, rhs);
      } else {
          return lhs == rhs;
      }
    };
    for (const auto &param: m_params) {
        if (cmp(param.first, key)) {
            out = param;
            break;
        }
    }

   return out;
}

void httb::base_request::setQuery(const std::string &key, const std::string &value, bool icase) {
    using toolboxpp::strings::equalsIgnoreCase;
    const auto &cmp = [icase](const std::string &lhs, const std::string &rhs) {
      if (icase) {
          return equalsIgnoreCase(lhs, rhs);
      } else {
          return lhs == rhs;
      }
    };
    for (auto &param: m_params) {
        if (cmp(param.first, key)) {
            param.second = value;
        }
    }
}

void httb::base_request::setQuery(const httb::kv &kv, bool icase) {
    setQuery(kv.first, kv.second, icase);
}

bool httb::base_request::removeQueryArray(const std::string &key, size_t index, bool icase) {
    using toolboxpp::strings::equalsIgnoreCase;
    const auto &cmp = [icase](const std::string &lhs, const std::string &rhs) {
      if (icase) {
          return equalsIgnoreCase(lhs, rhs);
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

bool httb::base_request::removeQuery(const std::string &key, bool icase) {
    using toolboxpp::strings::equalsIgnoreCase;
    const auto &cmp = [icase](const std::string &lhs, const std::string &rhs) {
      if (icase) {
          return equalsIgnoreCase(lhs, rhs);
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

std::vector<std::string> httb::base_request::getQueryArray(const std::string &key, bool icase) const {
    std::vector<std::string> out;

    using toolboxpp::strings::equalsIgnoreCase;
    const auto &cmp = [icase](const std::string &lhs, const std::string &rhs) {
      if (icase) {
          return equalsIgnoreCase(lhs, rhs);
      } else {
          return lhs == rhs;
      }
    };

    for (const auto &param: m_params) {
        if (cmp(param.first, key)) {
            out.push_back(param.second);
        }
    }

    return out;
}

std::string httb::base_request::getQueryString() const {
    std::string combined;
    if (!m_params.empty()) {
        std::stringstream ss;
        std::vector<std::string> enc;
        for (auto &p: m_params) {
            ss << p.first << "=" << p.second;
            enc.push_back(ss.str());
            ss.str("");
            ss.clear();
        }
        std::string glued = toolboxpp::strings::glue("&", enc);
        combined += "?" + glued;
    }

    return combined;
}

const httb::kv_vector &httb::base_request::getQueryList() const {
    return m_params;
}

std::string httb::base_request::getHost() const {
    return m_host;
}

void httb::base_request::setHost(const std::string &hostname) {
    m_host = hostname;
}

uint16_t httb::base_request::getPort() const {
    const int port = std::stoi(m_port);
    return (uint16_t) port;
}

void httb::base_request::setPort(uint16_t portNumber) {
    std::stringstream ss;
    ss << portNumber;
    m_port = ss.str();
}

std::string httb::base_request::getPortString() const {
    return m_port;
}

std::string httb::base_request::getProtocolName() const {
    return m_proto;
}

void httb::base_request::setProtocolName(const std::string &protocolName) {
    m_proto = protocolName;
}

std::string httb::base_request::getPath() const {
    if(m_path.empty()) {
        return "/";
    }
    return m_path;
}

std::string httb::base_request::getPathWithQuery() const {
    return getPath() + getQueryString();
}

void httb::base_request::setPath(const std::string &path) {
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

void httb::base_request::addPath(const std::string &path) {
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

bool httb::base_request::isSSL() const {
    return m_ssl;
}

boost::beast::http::request<boost::beast::http::string_body> httb::request::createBeastRequest() const {
    namespace http = boost::beast::http;

    http::request<http::string_body> req{getMethod(), getPathWithQuery(), 11};
    req.set(http::field::host, getHost());

    std::stringstream versionBuilder;
    versionBuilder << "httb/" << HTTB_VERSION;
    versionBuilder << " (boost " << BOOST_VERSION << ")";
    req.set(http::field::user_agent, versionBuilder.str());
    req.set(http::field::accept, "*/*");
    req.set(http::field::content_length, "0");

    for (const auto &field: getHeaders()) {
        req.set(field.first, field.second);
    }

    if (hasBody()) {
        req.body() = getBody();
        req.prepare_payload();
    }

    return req;
}
void httb::request::setBody(const std::string &body) {
    io_container::setBody(body);
}
void httb::request::setBody(std::string &&body) {
    io_container::setBody(body);
}
void httb::request::setBody(const httb::request_body &body) {
    io_container::setBody(body.build(this));
}
void httb::request::setBody(httb::request_body &&body) {
    std::string builtBody = body.build(this);
    io_container::setBody(std::move(builtBody));
}
