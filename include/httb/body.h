/*!
 * httb.
 * body.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_BODY_H
#define HTTB_BODY_H

#include "httb/httb_config.h"
#include "httb/io_container.h"

#include <string>

namespace httb {

class HTTB_API request_body {
public:
    virtual ~request_body() = default;
    virtual std::string build(httb::io_container* request) const = 0;
};

} // namespace httb

#endif //HTTB_BODY_H
