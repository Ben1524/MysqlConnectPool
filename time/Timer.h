/**
* @ClassName Timer
* @Author cxk
* @Data 25-6-3 上午12:17
*/

#ifndef MYSQLCONNECTPOOL_TIMER_H
#define MYSQLCONNECTPOOL_TIMER_H

#include <functional>
#include <atomic>
#include <chrono>
#include "NonCopyable.h"

namespace cxk
{
using TimerCallback = std::function<void()>;
using TimerId = uint64_t;
using TimePoint = std::chrono::steady_clock::time_point;
using TimeInterval = std::chrono::microseconds;

/**
 * @class Timer
 * @brief 定时器类，用于管理单次或周期性定时事件
 * @note 继承自NonCopyable，禁止拷贝构造和赋值，确保资源唯一性
 */
class Timer : public NonCopyable
{
public:
    Timer(const TimerCallback &cb, const TimePoint &when, const TimeInterval &interval);
    Timer(TimerCallback &&cb, const TimePoint &when, const TimeInterval &interval);
    template<class Func,typename...Arg>
    explicit Timer(Func&&f,Arg&&...arg,const TimePoint &when, const TimeInterval &interval);

    ~Timer();

    void run() const;
    void restart(const TimePoint &now);
    bool operator<(const Timer &t) const;
    bool operator>(const Timer &t) const;

    const TimePoint &when() const;
    bool isRepeat();
    TimerId id();

private:
    TimerCallback callback_;
    TimePoint when_;
    const TimeInterval interval_;
    const bool repeat_;
    const TimerId id_;

    static std::atomic<TimerId> timersCreated_;
};


template<class Func, typename... Arg>
Timer::Timer(Func &&f, Arg &&... arg, const TimePoint &when, const TimeInterval &interval)
    : callback_(std::bind(std::forward<Func>(f), std::forward<Arg>(arg)...)),
      when_(when),
      interval_(interval),
      repeat_(interval.count() > 0),
      id_(++timersCreated_)
{
}


} // namespace cxk

#endif // MYSQLCONNECTPOOL_TIMER_H