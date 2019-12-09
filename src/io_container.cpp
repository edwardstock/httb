/*!
 * httb.
 * io_container.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "httb/io_container.h"

#include "utils.h"

#include <algorithm>
#include <toolbox/strings.hpp>

// BASE IO
httb::io_container::io_container()
    : m_body() {
}

void httb::io_container::set_body(const std::string& data) {
    m_body = data;
    set_header({"Content-Length", httb::to_string(m_body.length())});
}
void httb::io_container::set_body(std::string&& data) {
    m_body = std::move(data);
    set_header({"Content-Length", httb::to_string(m_body.length())});
}
void httb::io_container::set_header(httb::kv&& key_value) {
    using toolbox::strings::equals_icase;
    bool found = false;
    for (auto& kv : m_headers) {
        if (equals_icase(kv.first, key_value.first)) {
            kv.second = key_value.second;
            found = true;
        }
    }

    if (!found) {
        return add_header(std::move(key_value));
    }
}
bool httb::io_container::has_header(const std::string& name) const {
    using toolbox::strings::equals_icase;
    for (auto& h : m_headers) {
        if (equals_icase(h.first, name)) {
            return true;
        }
    }

    return false;
}

boost::optional<httb::kv> httb::io_container::find_header_pair(const std::string& name) const {
    boost::optional<httb::kv> out;
    using toolbox::strings::equals_icase;
    for (auto& h : m_headers) {
        if (equals_icase(h.first, name)) {
            out = h;
            break;
        }
    }

    return out;
}
std::string httb::io_container::get_header_value(const std::string& headerName) const {
    using toolbox::strings::equals_icase;
    for (auto& h : m_headers) {
        if (equals_icase(h.first, headerName)) {
            return h.second;
        }
    }

    return std::string();
}
bool httb::io_container::cmp_header_value(const std::string& header_name, const std::string& comparable) const {
    if (!has_header(header_name))
        return false;
    return get_header_value(header_name) == comparable;
}
void httb::io_container::add_header(const std::string& name, const std::string& value) {
    using toolbox::strings::equals_icase;
    for (auto& h : m_headers) {
        if (equals_icase(h.first, name)) {
            h.second = value;
            return;
        }
    }
    m_headers.emplace_back(toolbox::strings::to_lower_case(name), value);
}
void httb::io_container::add_header(const httb::kv& kv) {
    add_header(kv.first, kv.second);
}
void httb::io_container::add_header(httb::kv&& kv) {
    using toolbox::strings::equals_icase;
    for (auto& h : m_headers) {
        if (equals_icase(h.first, kv.first)) {
            h.second = std::move(kv.second);
            return;
        }
    }
    m_headers.push_back(std::move(kv));
}
void httb::io_container::add_headers(const httb::kv_vector& values) {
    m_headers.reserve(m_headers.size() + values.size());
    std::for_each(values.begin(), values.end(), [this](const httb::kv& pair) {
        add_header(pair.first, pair.second);
    });
}

bool httb::io_container::remove_header(const std::string& name, bool icase) {
    using toolbox::strings::equals_icase;
    bool removed = false;
    size_t i = 0;
    for (auto& h : m_headers) {
        if (icase && equals_icase(h.first, name)) {
            m_headers.erase(m_headers.begin() + i);
            removed = true;
        } else if (h.first == name) {
            m_headers.erase(m_headers.begin() + i);
            removed = true;
        }

        i++;
    }

    return removed;
}

void httb::io_container::clear_headers() {
    m_headers.clear();
}
size_t httb::io_container::headers_size() const {
    return m_headers.size();
}

void httb::io_container::set_headers(const httb::icase_map_t& map) {
    m_headers.reserve(m_headers.size() + map.size());
    for (auto& h : map) {
        add_header(h.first, h.second);
    }
}
void httb::io_container::set_headers(const httb::icase_multimap_t& mmp) {
    m_headers.reserve(m_headers.size() + mmp.size());
    for (auto& h : mmp) {
        add_header(h.first, h.second);
    }
}
std::string httb::io_container::get_body() const {
    return m_body;
}
const char* httb::io_container::get_body_c() const {
    return m_body.c_str();
}
std::size_t httb::io_container::get_body_size() const {
    return m_body.length();
}

void httb::io_container::clear_body() {
    m_body.clear();
}

bool httb::io_container::has_body() const {
    return !m_body.empty();
}
bool httb::io_container::has_headers() const {
    return !m_headers.empty();
}
const httb::kv_vector& httb::io_container::get_headers() const {
    return m_headers;
}
std::vector<std::string> httb::io_container::get_headers_glued() const {
    std::vector<std::string> out(m_headers.size());

    std::stringstream ss;
    int i = 0;
    for (const auto& h : m_headers) {
        ss << h.first << ": " << h.second;
        out[i] = ss.str();
        ss.str("");
        ss.clear();
        i++;
    }

    return out;
}
