#include "DeadlineTimer.h"


deadline_timer::deadline_timer() 
    : _execute(false){}

void deadline_timer::start(int interval, std::function<void(void)> func) {
    _execute = true;
    std::thread([=]()
        {
            while (_execute) {
                func();
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(interval));
            }
        }).detach();
}

void deadline_timer::stop()
{
    _execute = false;
}

