/*!
 * httb.
 * utils.h
 *
 * \date 12/09/2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#ifndef HTTB_UTILS_CPP
#define HTTB_UTILS_CPP

#include "httb/response.h"

#include <algorithm>
#include <boost/asio/ssl/context.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <random>
#include <sstream>
#include <string>
#include <toolbox/io.h>
#include <toolbox/strings.hpp>
#include <unordered_map>
#include <vector>

namespace httb {

static void scan_dir(const boost::filesystem::path& path,
                     std::unordered_map<std::string, std::string>& map) {
    namespace fs = boost::filesystem;

    if (!fs::exists(path)) {
        return;
    }

    if (fs::is_regular_file(path)) {
        const std::string ext = path.extension().generic_string();
        if (toolbox::strings::equals_icase(".crt", ext) || toolbox::strings::equals_icase(".pem", ext)) {
            map[path.generic_string()] = toolbox::io::file_read_full(path.generic_string());
        }
    } else if (fs::is_directory(path)) {
        for (fs::directory_entry& x : fs::directory_iterator(path)) {
            scan_dir(x.path(), map);
        }
    } else {
        // irregular for or directory
    }
}

static void load_root_certs(boost::asio::ssl::context& ctx, boost::system::error_code& ec) {
    const std::vector<std::string> certPaths = {
        "/etc/ssl/certs/ca-certificates.crt",                // Debian/Ubuntu/Gentoo etc.
        "/etc/pki/tls/certs",                                // Fedora/RHEL
        "/etc/pki/tls/certs/ca-bundle.crt",                  // Fedora/RHEL 6
        "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", // CentOS/RHEL 7
        "/etc/ssl/ca-bundle.pem",                            // OpenSUSE
        "/etc/pki/tls/cacert.pem",                           // OpenELEC
        "/etc/ssl/certs",                                    // SLES10/SLES11, https://golang.org/issue/12139
        "/etc/ssl/cert.pem",                                 // macOS Common
        "/usr/local/etc/openssl/certs",                      // macOS OpenSSL
        "/usr/local/etc/openssl@1.1/certs",                  // macOS OpenSSL 1.1
        "/system/etc/security/cacerts",                      // Android
        "/usr/local/share/certs",                            // FreeBSD
        "/etc/openssl/certs",                                // NetBSD
    };

    namespace fs = boost::filesystem;
    std::unordered_map<std::string, std::string> certFiles;
    for (const auto& path : certPaths) {
        try {
            scan_dir(fs::path(path), certFiles);
        } catch (const std::exception& e) {
        }
    }

    for (auto& iter : certFiles) {
        ctx.add_certificate_authority(
            boost::asio::buffer(iter.second.data(), iter.second.size()), ec);
    }
}

static void load_root_certs(boost::asio::ssl::context& ctx) {
    boost::system::error_code ec;
    load_root_certs(ctx, ec);
    if (ec) {
        throw boost::system::system_error{ec};
    }
}

static httb::response boost_err_to_rep_err(httb::response&& in, boost::system::error_code ec) {
    httb::response out = in;
    out.code = httb::response::INTERNAL_ERROR_OFFSET + ec.value();
    std::stringstream errorStream;
    errorStream << ec.category().name() << "[" << out.code << "]"
                << ": " << ec.message();
    out.set_body(errorStream.str());

    return out;
}

static httb::response boost_err_to_rep_err(httb::response&& in, const boost::system::system_error& e) {
    return boost_err_to_rep_err(std::move(in), e.code());
}

static std::string gen_random_string(int length) {
    std::string chars(
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "1234567890");

    std::random_device rng;
    std::uniform_int_distribution<> index_dist(0, static_cast<int>(chars.size() - 1));
    std::stringstream ss;
    for (int i = 0; i < length; ++i) {
        ss << chars[index_dist(rng)];
    }

    return ss.str();
}

/// \brief Integral and floating types to string
/// \tparam T is_integral<T> or is_floating_point<T> required
/// \param n Value
/// \return string representation
template<typename T>
std::string to_string(T n) {
    static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value,
                  "Value can be only integral type or floating point");

    std::stringstream ss;
    ss << n;

    return ss.str();
}

} // namespace httb

#endif //HTTB_UTILS_CPP
