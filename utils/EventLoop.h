/**
*@ClassName EventLoop
*@Author cxk
*@Data 25-6-2 下午10:15
*/
//

#ifndef MYSQLCONNECTPOOL_EVENTLOOP_H
#define MYSQLCONNECTPOOL_EVENTLOOP_H
#include "MPSCQueue.h"
#include <NonCopyable.h>
#include <thread>
#include <memory>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <chrono>
#include <limits>
#include <atomic>

namespace cxk
{
class EventDispatcher;
class Poller; // 前向声明Poller类
class TimerQueue;
using EventDispatcherList = std::vector<EventDispatcher *>;
using Func = std::function<void()>;
using TimerId = std::size_t;
enum
{
    InvalidTimerId = 0
};

/**
 * @brief 顾名思义，此类表示在特定线程中运行的事件循环。
 * 该事件循环可以异步处理网络I/O事件和定时器事件。
 * @note 一个事件循环对象始终属于一个独立的线程，且一个线程中最多有一个事件循环对象。
 * 我们可以称事件循环对象属于其所在的线程，或称该线程为事件循环的线程。
 */
class EventLoop : public NonCopyable
{
private:
    void abortNotInLoopThread(); // 不是在指定的事件循环线程中终止
    void wakeup(); // 唤醒事件循环线程
    void wakeupRead(); // 唤醒读事件
    std::atomic<bool> looping_; // 事件循环是否正在运行
    std::thread::id threadId_; // 所属线程的ID
    std::atomic<bool> quit_; // 退出标志
    std::unique_ptr<Poller> poller_; // 事件轮询器 ，linux使用epoll实现

    EventDispatcherList activeDispatchers_; // 触发的事件分发器列表
    EventDispatcher *currentActiveDispatcher_; // 当前活跃的事件分发器
    bool eventHandling_; // 是否正在处理事件
    MPSCQueue<Func> funcs_; // 待执行的函数队列（多生产者单消费者）
    std::unique_ptr<TimerQueue> timerQueue_; // 定时器队列
    MPSCQueue<Func> funcsOnQuit_; // 退出时待执行的函数队列
    bool callingFuncs_{false}; // 是否正在调用函数
    int wakeupFd_; // 唤醒文件描述符（Linux专用）
    std::unique_ptr<EventDispatcher> wakeupDispatcherPtr_; // 唤醒事件分发器

    void doRunInLoopFuncs(); // 执行事件循环中的待处理函数
    size_t index_{std::numeric_limits<size_t>::max()}; // 事件循环索引（默认最大值）
    EventLoop **threadLocalLoopPtr_; // 线程局部存储的事件循环指针
};
}



#endif //MYSQLCONNECTPOOL_EVENTLOOP_H
