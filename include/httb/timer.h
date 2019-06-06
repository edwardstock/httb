/*!
 * httb.
 * timer
 *
 * \date 25.05.19
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef HTTB_TIMER_H
#define HTTB_TIMER_H

#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <mutex>
#include <atomic>
#include <condition_variable>

class timer {
 public:
    timer(std::chrono::seconds time, const std::function<void(void)>& f);
    ~timer();
    void cancel();
 private:
    std::atomic_bool canceled;
    void wait_then_call();
    std::mutex mtx;
    std::condition_variable cv{};
    std::chrono::milliseconds time;
    std::function <void(void)> f;
    std::thread wait_thread{[this]() {wait_then_call(); }};
};


#endif //HTTB_TIMER_H
