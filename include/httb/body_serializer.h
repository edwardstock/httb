/*!
 * httb.
 * body_serializer.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_BODY_SERIALIZER_H
#define HTTB_BODY_SERIALIZER_H

#include <string>

namespace httb {
class body_string_serializer {
public:
    virtual std::string serialize(const std::string &raw) const = 0;

};
}

#endif //HTTB_BODY_SERIALIZER_H
