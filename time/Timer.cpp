/**
*@ClassName Timer
*@Author cxk
*@Data 25-6-3 上午12:17
*/
//

#include <stdexcept>
#include <spdlog/spdlog.h>
#include "Timer.h"

namespace cxk
{
std::atomic<TimerId> Timer::timersCreated_ = {0}; // 静态成员初始化

Timer::Timer(const TimerCallback &cb, const TimePoint &when, const TimeInterval &interval)
        : callback_(cb),
          when_(when),
          interval_(interval),
          repeat_(interval.count() > 0),
          id_(++timersCreated_)
{
    // 空实现
}

Timer::Timer(TimerCallback &&cb, const TimePoint &when, const TimeInterval &interval)
        : callback_(std::move(cb)),
          when_(when),
          interval_(interval),
          repeat_(interval.count() > 0),
          id_(++timersCreated_)
{
    // 空实现
}

Timer::~Timer()
{
    spdlog::debug("Timer {} destroyed", id_);
}

void Timer::run() const
{
    if (callback_)
    {
        callback_();
    }
    else
    {
        // 如果没有设置回调函数，可以选择抛出异常或记录日志
         throw std::runtime_error("Timer callback is not set");
    }
}

void Timer::restart(const TimePoint &now)
{
    if (repeat_)
    {
        when_ = now + interval_;
    }
    else
        when_ = std::chrono::steady_clock::now();
}

bool Timer::operator<(const Timer &t) const
{
    return when_ < t.when_;
}

bool Timer::operator>(const Timer &t) const
{
    return when_ > t.when_;
}

const TimePoint &Timer::when() const
{
    return when_;
}

bool Timer::isRepeat()
{
    return repeat_;
}

TimerId Timer::id()
{
    return id_;
}

} // namespace cxk