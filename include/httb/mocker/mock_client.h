/*!
 * httb.
 * mock_client.h
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#ifndef HTTB_MOCK_CLIENT_H
#define HTTB_MOCK_CLIENT_H

#include "httb/client.h"

#include <functional>

namespace httb {

using request_executor = std::function<response(const request&)>;

class mock_client : public client {
public:
    mock_client(request_executor executor)
        : client(), m_executor(std::move(executor)) {
    }

    ~mock_client() override {
    }

    response execute_blocking(const request& request) override {
        if (!m_executor) {
            throw std::runtime_error("Mock executor did not set.");
        }

        std::lock_guard<std::mutex> lock(m_executor_lock);
        return m_executor(request);
    }
    void execute(const request& request, const response_func_t& cb, const progress_func_t& = nullptr) override {
        if (!m_executor) {
            throw std::runtime_error("Mock executor did not set.");
        }
        net::io_context ctx(1);
        net::post(ctx, [this, &request, &cb]() {
            if (!cb) {
                return;
            }
            std::lock_guard<std::mutex> lock(m_executor_lock);
            cb(m_executor(request));
        });

        ctx.run();
    }
    void execute_in_context(net::io_context& ioc, const request& request, const response_func_t& cb, const progress_func_t& onProgress = nullptr) override {
        if (!m_executor) {
            throw std::runtime_error("Mock executor did not set.");
        }

        net::post(ioc, [this, &request, &cb]() {
            if (!cb) {
                return;
            }
            std::lock_guard<std::mutex> lock(m_executor_lock);
            cb(m_executor(request));
        });

        ioc.run();
    }

protected:
    request_executor m_executor;
    std::mutex m_executor_lock;
};

} // namespace httb

#endif //HTTB_MOCK_CLIENT_H
