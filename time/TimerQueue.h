/**
*@ClassName TimerQueue
*@Author cxk
*@Data 25-6-2 下午11:25
*/
//

#ifndef MYSQLCONNECTPOOL_TIMERQUEUE_H
#define MYSQLCONNECTPOOL_TIMERQUEUE_H
#include "Timer.h"
#include <memory>
#include <ctime>
#include <atomic>
#include <unordered_set>
#include <functional>
#include <queue>
namespace cxk
{
class EventLoop;
class EventDispatcher;

using TimerPtr = std::shared_ptr<Timer>;

/**
 * @brief 定时器指针比较器，用于优先队列排序
 * 按照定时器触发时间倒序排列（最早触发的定时器位于队顶）
 */
struct TimerPtrComparer
{
    bool operator()(const TimerPtr &x, const TimerPtr &y) const
    {
        return *x > *y; // 利用Timer的operator>实现倒序排序
    }
};
class TimerQueue:public NonCopyable
{
public:
    explicit TimerQueue(EventLoop*loop);
    ~TimerQueue();
    TimerId addTimer(const TimerCallback &cb,
                     const TimePoint &when,
                     const TimeInterval &interval);
    TimerId addTimer(TimerCallback &&cb,
                     const TimePoint &when,
                     const TimeInterval &interval);
    void addTimerInLoop(const TimerPtr &timer);
    void invalidateTimer(TimerId id);
    void reset();

protected:
    EventLoop *loop_;
#ifdef __linux__
    int timerfd_;
    std::shared_ptr<EventDispatcher> timerfdEventDispatcherPtr_;
    void handleRead();
#endif
    std::priority_queue<TimerPtr, std::vector<TimerPtr>, TimerPtrComparer>
            timers_;

    bool callingExpiredTimers_;
    bool insert(const TimerPtr &timePtr);
    std::vector<TimerPtr> getExpired();
    void reset(const std::vector<TimerPtr> &expired, const TimePoint &now);
    std::vector<TimerPtr> getExpired(const TimePoint &now);

private:
    std::unordered_set<uint64_t> timerIdSet_;
};

}



#endif //MYSQLCONNECTPOOL_TIMERQUEUE_H
