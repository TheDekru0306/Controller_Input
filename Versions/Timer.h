#ifndef DEBBUGJSINPUT_TIMER_H
#define DEBBUGJSINPUT_TIMER_H


#include <cstdint>
#include <functional>
#include <chrono>

class Timer
{
public:
    Timer();
    void Start(int64_t timeout, std::function<void()> func);
    void Update();
    void Reset();
    void onFinish(std::function<void()> func);
    Timer(const Timer&) = delete;
    Timer(const Timer&&) = delete;
    Timer & operator=(const Timer&) = delete;
private:
    bool finished;
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
    std::function<void()> callback, on_finished;
    bool initialized = false, hasFinishedCallback = false;
    int64_t timeout;
};


#endif //DEBBUGJSINPUT_TIMER_H
