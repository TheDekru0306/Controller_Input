#include "Timer.h"

Timer::Timer()
{

}

void Timer::Start(int64_t timeout, std::function<void()> func) {
    finished = false;
    initialized = true;

    this->startTime = std::chrono::high_resolution_clock::now();
    this->endTime = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds (timeout);
    this->callback = func;
    this->timeout = timeout;
}

void Timer::Update()
{
    if(!initialized){
        return;
    }
    if(!finished){
        callback();
    }
    if (!finished && this->startTime >= this->endTime)
    {
        finished = true;
        if(hasFinishedCallback){
            on_finished();
        }
    }
    this->startTime = std::chrono::high_resolution_clock::now();
}

void Timer::Reset()
{
    finished = false;
    this->startTime = std::chrono::high_resolution_clock::now();
    this->endTime = std::chrono::high_resolution_clock::now() + std::chrono::seconds(timeout);
}

void Timer::onFinish(std::function<void()> func) {
    on_finished = func;
    hasFinishedCallback = true;
}

