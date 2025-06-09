/**
*@ClassName TimerQueue
*@Author cxk
*@Data 25-6-2 下午11:25
*/
//

#include "event/EventLoop.h"
#include "event/EventDispatcher.h"
#include "TimerQueue.h"
#include <sys/timerfd.h>
#include <string.h>
#include <iostream>
#include "TimerQueue.h"

#include <unistd.h>
#include <spdlog/spdlog.h>
using namespace cxk;

static int createTimerfd()
{
    // TFD_NONBLOCK 使得timerfd在非阻塞模式下工作，TFD_CLOEXEC确保在fork时不会继承该文件描述符
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC); // CLOCK_MONOTONIC 确保定时器基于单调时钟
    if (timerfd < 0)
    {
        std::cerr << "create timerfd failed!" << std::endl;
    }
    return timerfd;
}


static struct timespec howMuchTimeFromNow(const TimePoint &when)
{
    auto microSeconds = std::chrono::duration_cast<std::chrono::microseconds>(
                            when - std::chrono::steady_clock::now())
                            .count();  // 计算从现在到指定时间点的微秒数
    if (microSeconds < 100) // 最小间隔为100微秒
    {
        microSeconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microSeconds / 1000000); // 将微秒转换为秒
    ts.tv_nsec = static_cast<long>((microSeconds % 1000000) * 1000);
    return ts;
}

static void resetTimerfd(int timerfd, const TimePoint &expiration)
{
    // 重置timerfd以通知事件循环
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof(newValue));
    memset(&oldValue, 0, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration); // 设置定时器的初始值
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue); // timerfd_settime用于更新定时器文件描述符的下次触发事件，返回旧事件
    if (ret)
    {
        std::cerr << "timerfd_settime() failed!" << std::endl;
    }
}

static void readTimerfd(int timerfd,const TimePoint&)
{
    uint64_t howMany;
    std::size_t n = ::read(timerfd, &howMany, sizeof(howMany)); // 读取timerfd的值
    if (n != sizeof(howMany))
    {
        spdlog::error("read timerfd failed, n = {}, errno = {}", n, errno);
    }
}


TimerQueue::TimerQueue(cxk::EventLoop *loop):
loop_(loop),timerfd_(createTimerfd()),timerfdEventDispatcherPtr_(std::make_shared<EventDispatcher>(loop, timerfd_)),
timers_(),callingExpiredTimers_(false)
{
    timerfdEventDispatcherPtr_->setReadCallback([this]() { this->handleRead(); }); // 等价于直接调用成员函数,C++编译器会自动转换
    // 启用读取事件，timerfd可读时会触发handleRead
    timerfdEventDispatcherPtr_->enableReading();
}

TimerQueue::~TimerQueue()
{
    auto dispatcher = timerfdEventDispatcherPtr_;
    auto fd = timerfd_;
    loop_->runInLoop([dispatcher, fd]() {
        dispatcher->disableAll(); // 禁用所有事件
        dispatcher->remove(); // 从事件循环中移除
        ::close(fd); // 关闭timerfd文件描述符
    });
}

TimerId TimerQueue::addTimer(const TimerCallback& cb, const TimePoint& when, const TimeInterval& interval)
{
    auto timePtr= std::make_shared<Timer>(cb, when, interval);
    loop_->runInLoop([this, timePtr]() { this->addTimerInLoop(timePtr); });
    return timePtr->id();
}

TimerId TimerQueue::addTimer(TimerCallback&& cb, const TimePoint& when, const TimeInterval& interval)
{
    auto timePtr = std::make_shared<Timer>(std::move(cb), when, interval);
    loop_->runInLoop([this, timePtr]() { this->addTimerInLoop(timePtr); });
    return timePtr->id();
}

// 添加时可能会更新最早应被触发的定时器
void TimerQueue::addTimerInLoop(const TimerPtr& timer)
{
    loop_->assertInLoopThread(); // 确保在EventLoop的线程中调用
    timerIdSet_.insert(timer->id()); // 将定时器ID添加到集合中
    bool earliestChanged = insert(timer); // 将定时器插入到优先队列中
    if (earliestChanged)
    {
        // 如果最早的定时器发生了变化，重置timerfd以通知事件循环
        resetTimerfd(timerfd_, timer->when());
    }
}

void TimerQueue::invalidateTimer(TimerId id)
{
    loop_->runInLoop([id, this]() {
        this->timerIdSet_.erase(id);
    });
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    const auto now = std::chrono::steady_clock::now(); // 获取当前时间点
    readTimerfd(timerfd_, now); // 读取timerfd的值，清除事件，避免事件一直触发

    auto expired = getExpired(now); // 获取所有已到期的定时器
    callingExpiredTimers_ = true; // 标记正在处理已到期的定时器
    for (const auto& timerPtr : expired) // 遍历所有已到期的定时器
    {
        if (timerIdSet_.find(timerPtr->id()) != timerIdSet_.end()) // 检查定时器ID是否存在或者有效
        {
            timerPtr->run(); // 执行定时器的回调函数
        }
    }
    callingExpiredTimers_ = false; // 重置标记
    reset(expired, now); // 重置已到期的定时器，可能会重新插入一些定时器，尝试清除已经过期的定时器

}

bool TimerQueue::insert(const TimerPtr& timePtr)
{
    loop_->assertInLoopThread(); // 确保在EventLoop的线程中调用
    bool earliestChanged = false;
    if (timers_.empty() || *timePtr < *timers_.top()) // 如果当前队列为空或新定时器比最早的定时器更早
    {
        earliestChanged = true; // 标记最早的定时器发生了变化
    }
    timers_.push(timePtr); // 将新定时器添加到优先队列中
    return earliestChanged; // 返回是否更新了最早的定时器
}

std::vector<TimerPtr> TimerQueue::getExpired()
{
    loop_->assertInLoopThread(); // 确保在EventLoop的线程中调用
    std::vector<TimerPtr> expired;
    while (!timers_.empty() && timers_.top()->when() <= std::chrono::steady_clock::now()) // 检查最早的定时器是否已到期
    {
        expired.push_back(timers_.top()); // 将到期的定时器添加到结果列表中
        timers_.pop(); // 从优先队列中移除已到期的定时器
    }
    return expired; // 返回所有已到期的定时器
}

std::vector<TimerPtr> TimerQueue::getExpired(const TimePoint &now)
{
    loop_->assertInLoopThread(); // 确保在EventLoop的线程中调用
    std::vector<TimerPtr> expired;
    while (!timers_.empty() && timers_.top()->when() <= now) // 检查最早的定时器是否已到期
    {
        expired.push_back(timers_.top()); // 将到期的定时器添加到结果列表中
        timers_.pop(); // 从优先队列中移除已到期的定时器
    }
    return expired; // 返回所有已到期的定时器
}

void TimerQueue::reset()
{
    loop_->runInLoop([this]{
        timerfdEventDispatcherPtr_->disableAll();
        timerfdEventDispatcherPtr_->remove(); // 从事件循环中移除
        close(timerfd_);
        timerfd_ = createTimerfd(); // 重新创建timerfd
        timerfdEventDispatcherPtr_ = std::make_shared<EventDispatcher>(loop_, timerfd_);
        timerfdEventDispatcherPtr_->setReadCallback([this]() { this->handleRead(); }); // 设置读取回调
        timerfdEventDispatcherPtr_->enableReading(); // 启用读取事件
        if (!timers_.empty()) // 如果还有未到期的定时器
        {
            resetTimerfd(timerfd_, timers_.top()->when()); // 重置timerfd以通知事件循环
        }
        });
}

void TimerQueue::reset(const std::vector<TimerPtr> &expired, const TimePoint &now)
{
    loop_->assertInLoopThread(); // 确保在EventLoop的线程中调用
    for (const auto &timer : expired) // 遍历所有已到期的定时器
    {
        auto iter = timerIdSet_.find(timer->id()); // 查找定时器ID
        if (iter != timerIdSet_.end()) // 如果定时器ID存在
        {
            if (timer->isRepeat())
            {
                timer->restart(now); // 重启定时器，
                insert(timer); // 将重启后的定时器重新插入到优先队列中
            }
            else
            {
                timerIdSet_.erase(iter);
            }
        }
    }
    if (!timers_.empty()) // 如果还有未到期的定时器
    {
        resetTimerfd(timerfd_, timers_.top()->when()); // 重置timerfd以通知事件循环
    }
}
  