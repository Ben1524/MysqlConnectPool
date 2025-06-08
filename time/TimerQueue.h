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
 * @brief TimerQueue 类用于管理和调度定时器事件
 *
 * 该类提供了在指定时间执行回调函数的功能，支持一次性和周期性定时器。
 * 它使用最小堆（优先队列）来高效地管理定时器，确保最快到期的定时器位于队列顶部。
 * 在Linux系统上，使用timerfd机制实现高精度定时器；在其他系统上使用自定义的超时处理逻辑。
 *
 * 主要功能包括：
 * - 添加新的定时器（一次性或周期性）
 * - 取消已注册的定时器
 * - 处理到期的定时器事件
 * - 管理定时器的生命周期
 *
 * 使用示例：
 * @code
 * // 在EventLoop中创建TimerQueue实例
 * TimerQueue timerQueue(loop);
 *
 * // 添加一个1秒后执行的一次性定时器
 * timerQueue.addTimer([]{ std::cout << "Timer fired!" << std::endl; },
 *                     TimePoint::now() + TimeInterval(1.0),
 *                     TimeInterval(0));
 *
 * // 添加一个每2秒执行一次的周期性定时器
 * timerQueue.addTimer([]{ std::cout << "Periodic timer fired!" << std::endl; },
 *                     TimePoint::now() + TimeInterval(2.0),
 *                     TimeInterval(2.0));
 * @endcode
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
    friend class EventLoop;
    /**
     * @brief 构造函数，创建一个TimerQueue实例
     * @param loop 关联的EventLoop对象，定时器事件将在该EventLoop中执行
     */
    explicit TimerQueue(EventLoop *loop);

    /**
     * @brief 析构函数，销毁TimerQueue实例并清理所有资源
     */
    ~TimerQueue();

    /**
     * @brief 添加一个新的定时器（左值引用版本）
     * @param cb 定时器到期时执行的回调函数
     * @param when 定时器首次触发的时间点
     * @param interval 定时器触发的时间间隔，为0表示一次性定时器
     * @return 返回一个TimerId对象，用于标识这个定时器，可用于取消操作
     */
    TimerId addTimer(const TimerCallback &cb,
                     const TimePoint &when,
                     const TimeInterval &interval);

    /**
     * @brief 添加一个新的定时器（右值引用版本，支持移动语义）
     * @param cb 定时器到期时执行的回调函数（右值引用）
     * @param when 定时器首次触发的时间点
     * @param interval 定时器触发的时间间隔，为0表示一次性定时器
     * @return 返回一个TimerId对象，用于标识这个定时器，可用于取消操作
     */
    TimerId addTimer(TimerCallback &&cb,
                     const TimePoint &when,
                     const TimeInterval &interval);

    /**
     * @brief 在EventLoop的线程中添加定时器（内部使用）
     * @param timer 要添加的定时器智能指针
     *
     * 此方法应在EventLoop的线程调用，用于安全地添加定时器
     */
    void addTimerInLoop(const TimerPtr &timer);

    /**
     * @brief 使指定的定时器失效（取消定时器）
     * @param id 要取消的定时器的ID
     *
     * 调用此方法后，对应的定时器将不会再触发，即使已经到期
     */
    void invalidateTimer(TimerId id);

protected:

    /**
    * @brief Linux系统上的timerfd文件描述符
    *
    * 用于实现高精度定时器事件通知
    */
    int timerfd_;
    /**
     * @brief 处理timerfd可读事件的回调函数
     *
     * 当timerfd可读时，表示有定时器到期，将调用此方法处理到期的定时器
     */
    void handleRead();

    EventLoop *loop_;
    std::shared_ptr<EventDispatcher> timerfdEventDispatcherPtr_;
    std::priority_queue<TimerPtr, std::vector<TimerPtr>, TimerPtrComparer>
            timers_;
    bool callingExpiredTimers_;
    bool insert(const TimerPtr &timePtr);
    std::vector<TimerPtr> getExpired();
    void reset(const std::vector<TimerPtr> &expired, const TimePoint &now);
    std::vector<TimerPtr> getExpired(const TimePoint &now);
    void reset();

private:
    /**
     * @brief 存储所有活跃定时器ID的集合
     *
     * 用于快速查找和验证定时器ID的有效性
     */
    std::unordered_set<uint64_t> timerIdSet_;
};

}



#endif //MYSQLCONNECTPOOL_TIMERQUEUE_H
