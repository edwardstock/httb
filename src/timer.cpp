/*!
 * httb.
 * timer
 *
 * \date 25.05.19
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "httb/timer.h"

timer::timer(std::chrono::seconds time, const std::function<void(void)> &f) :
    canceled(false),
    time{time},
    f{f} {

}
timer::~timer() {
    wait_thread.join();
}

void timer::cancel() {
    canceled = true;
}
void timer::wait_then_call() {
    std::unique_lock<std::mutex> lck{mtx};
    if(canceled) {
        return;
    }
    cv.wait_for(lck, time);
    if(canceled) {
        return;
    }
    f();
}
