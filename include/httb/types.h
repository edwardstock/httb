/*!
 * httb.
 * types.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_TYPES_H
#define HTTB_TYPES_H

#include <boost/asio/io_context.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace httb {

using request_body_type = boost::beast::http::string_body;
using response_body_type = boost::beast::http::dynamic_body;
using response_file_body_type = boost::beast::http::file_body;
using response_t = boost::beast::http::response<httb::response_body_type>;
using context = boost::asio::io_context;
/// \brief Error callback
using error_func_t = std::function<void(boost::system::error_code, std::string)>;
/// \brief Success callback
using success_func_t = std::function<void(httb::response_t&&, size_t)>;
/// \brief Progress callback
using progress_func_t = std::function<void(uint64_t, uint64_t, double)>;

/// \brief Simple std::pair<std::string, std::string>
using kv = std::pair<std::string, std::string>;
using kvd = std::pair<std::string, uint64_t>;

/// \brief Simple vector of pairs wss::web::KeyValue
using kv_vector = std::vector<httb::kv>;

inline bool case_insensitive_equal(const std::string& str1, const std::string& str2) noexcept {
    return str1.size() == str2.size() &&
           std::equal(str1.begin(), str1.end(), str2.begin(), [](char a, char b) {
               return tolower(a) == tolower(b);
           });
}
class icase_equal_t {
public:
    bool operator()(const std::string& str1, const std::string& str2) const noexcept {
        return case_insensitive_equal(str1, str2);
    }
};

// Based on https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/2595226#2595226
class icase_hash_t {
public:
    std::size_t operator()(const std::string& str) const noexcept {
        std::size_t h = 0;
        std::hash<int> hash;
        for (auto c : str)
            h ^= hash(tolower(c)) + 0x9e3779b9 + (h << 6u) + (h >> 2u);
        return h;
    }
};

using icase_multimap_t = std::unordered_multimap<std::string,
                                                 std::string,
                                                 icase_hash_t,
                                                 icase_equal_t>;

using icase_map_t = std::unordered_map<std::string,
                                       std::string,
                                       icase_hash_t,
                                       icase_equal_t>;

} // namespace httb

#endif //HTTB_TYPES_H
