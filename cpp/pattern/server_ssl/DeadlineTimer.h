#pragma once
#include <functional>
#include <chrono>
#include <future>
#include <cstdio>

class deadline_timer
{
public:
    deadline_timer();     
    void start(int interval, std::function<void(void)> func);
    void stop();
    
private:
    bool _execute;
};