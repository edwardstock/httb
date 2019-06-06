/**
 * wsserver
 * DateHelper.hpp
 *
 * @author Eduard Maximovich <edward.vstock@gmail.com>
 * @link https://github.com/edwardstock
 */

#ifndef HTTB_HELPERS_HPP
#define HTTB_HELPERS_HPP

#include <string>
#include <sstream>


namespace httb {
namespace utils {

/// \brief Integral and floating types to string
/// \tparam T is_integral<T> or is_floating_point<T> required
/// \param n Value
/// \return string representation
template<typename T>
const std::string toString(T n) {
    static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value,
                  "Value can be only integral type or floating point");

    std::stringstream ss;
    ss << n;

    return ss.str();
}
}
}

#endif //HTTB_HELPERS_HPP
