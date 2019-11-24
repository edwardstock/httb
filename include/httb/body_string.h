/*!
 * httb.
 * body_string.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_BODY_STRING_H
#define HTTB_BODY_STRING_H

#include "httb/body.h"

namespace httb {

/// \brief Simple body contained string.
/// Use for simple requests, like JSON, or plain
class body_string : public httb::request_body {
public:
    body_string(std::string body);
    virtual ~body_string();

    std::string build(httb::io_container *request) const override;

private:
    std::string m_body;
};

}

#endif // HTTB_BODY_STRING_H