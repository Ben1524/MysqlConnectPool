/**
*@ClassName EventLoop
*@Author cxk
*@Data 25-6-2 下午10:15
*/
//
#include <unistd.h>
#include <spdlog/spdlog.h>
#include "EventLoop.h"
#include "poll/EpollPoller.h"
#include "time/TimerQueue.h"
#include "EventDispatcher.h"
#include <sys/eventfd.h>
#include "utils/ScopeExit.h"

using namespace cxk;

static int createEventfd()
{
    // 参数0表示初始值为0
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC); // 创建一个事件文件描述符，EFD_NONBLOCK表示非阻塞模式，EFD_CLOEXEC表示在fork时不会继承该文件描述符
    if (evtfd < 0)
    {
        spdlog::error("Failed in eventfd: {}", strerror(errno));
        abort();
    }
    return evtfd;
}
const int kPollTimeMs = 10000;
thread_local EventLoop *t_loopInThisThread = nullptr;

EventLoop::EventLoop():
        looping_(false),
        threadId_(std::this_thread::get_id()),  // 记录创建是所属的线程ID
        quit_(false),
        poller_(EpollPoller::newPoller(this)),
        currentActiveDispatcher_(nullptr),
        eventHandling_(false),
        timerQueue_(new TimerQueue(this)),
        wakeupFd_(createEventfd()),
        wakeupDispatcherPtr_(new EventDispatcher(this, wakeupFd_)),
        threadLocalLoopPtr_(&t_loopInThisThread)
{
    if (t_loopInThisThread)
    {
        spdlog::error("There is already an EventLoop in this thread, you cannot create another one");
        exit(-1);
    }
    t_loopInThisThread = this; // 设置当前线程的事件循环指针为this，一个事件循环对应一个线程
    wakeupDispatcherPtr_->setReadCallback([this]
    {
        this->wakeupRead();
    });
    wakeupDispatcherPtr_->enableReading();
}




void EventLoop::abortNotInLoopThread()
{
    spdlog::error("EventLoop::abortNotInLoopThread");
    exit(1);
}

void EventLoop::wakeup()
{
    // if (!looping_)
    //     return;
    static uint64_t tmp = 1;
    int ret = write(wakeupFd_, &tmp, sizeof(tmp));  // 写入到文件描述中，这样就可以被epoll监听到
    (void)ret;
}

void EventLoop::wakeupRead()
{
    ssize_t ret = 0;
    uint64_t tmp;
    ret = read(wakeupFd_, &tmp, sizeof(tmp));
    if (ret < 0)
    {
        spdlog::error("EventLoop::wakeupRead read error: {}", strerror(errno));
    }
    else if (ret == 0)
    {
        spdlog::error("EventLoop::wakeupRead read zero bytes");
    }
    else
    {
        spdlog::debug("EventLoop::wakeupRead read {} bytes", ret);
    }
}

void EventLoop::doRunInLoopFuncs()
{
    callingFuncs_ = true;
    {
        // 确保标志在函数执行完毕后被清除，无论是否发生异常
        auto callingFlagCleaner =
            utils::makeScopeExit([this]() { callingFuncs_ = false; });

        // 临时存储异常，确保所有函数都被执行
        std::exception_ptr firstException;

        // 处理队列中的所有函数
        while (!funcs_.empty())
        {
            Func func;
            while (funcs_.dequeue(func))
            {
                try
                {
                    func();
                }
                catch (...)
                {
                    // 捕获第一个异常
                    if (!firstException)
                    {
                        firstException = std::current_exception();
                    }
                    // 继续处理其他函数
                }
            }
        }
        // 如果有异常，重新抛出第一个捕获的异常
        if (firstException)
        {
            std::rethrow_exception(firstException);
        }
    }
}



void EventLoop::assertInLoopThread()
{
    if (!isInLoopThread())
    {
        abortNotInLoopThread();
    }
}




void EventLoop::loop()
{
    assert(!looping_.load(std::memory_order_acquire)); // 确保没有重复调用loop
    assertInLoopThread();
    looping_.store(true, std::memory_order_release); // release语确保
    quit_.store(false, std::memory_order_release);

    std::exception_ptr loopException;

    try
    {
        auto loopFlagCleaner = utils::makeScopeExit(
                    [this]() { looping_.store(false, std::memory_order_release); });
        while (!quit_.load(std::memory_order_acquire))
        {
            activeDispatchers_.clear(); // 清空活跃的事件分发器列表
            poller_->poll(kPollTimeMs, &activeDispatchers_); // 轮询事件
            eventHandling_=true;
            for (auto& dispatcher : activeDispatchers_)
            {
                currentActiveDispatcher_ = dispatcher; // 设置当前活跃的事件分发器
                currentActiveDispatcher_->handleEvent(); // 处理事件
            }
            currentActiveDispatcher_ = nullptr; // 重置当前活跃的事件分发器
            eventHandling_ = false; // 事件处理结束
            doRunInLoopFuncs(); // 执行在事件循环中注册的函数
        }
    }catch (std::exception &e)
    {
        spdlog::error("Exception in EventLoop::loop: {}", e.what());
        loopException = std::current_exception();
    }

    // 上述循环出现异常被退出
    Func f;
    while (funcsOnQuit_.dequeue(f)) // 执行退出时注册的函数
    {
        f();
    }
    t_loopInThisThread = nullptr; // 取消限制
    if (loopException)
    {
        spdlog::error("Rethrowing exception from EventLoop::loop");
        std::rethrow_exception(loopException);
    }
}

void EventLoop::quit()
{
    quit_.store(true, std::memory_order_release);

    if (!isInLoopThread()) // 如果不是在对应线程里
    {
        wakeup();
    }
}

bool EventLoop::isInLoopThread() const
{
    return threadId_ == std::this_thread::get_id();
}

std::thread::id EventLoop::threadId() const
{
    return threadId_;
}

EpollPoller* EventLoop::poller() const
{
    assert(poller_ != nullptr);
    return poller_.get();
}

TimerQueue* EventLoop::timerQueue() const
{
    assert(timerQueue_ != nullptr);
    return timerQueue_.get();
}

void EventLoop::resetTimerQueue()
{
    assertInLoopThread();
    assert(!looping_.load(std::memory_order_acquire));
    timerQueue_->reset();
}

void EventLoop::queueInLoop(const Func &cb)
{
    funcs_.enqueue(cb);
    if (!isInLoopThread() || !looping_.load(std::memory_order_acquire))
    {
        wakeup();
    }
}
void EventLoop::queueInLoop(Func &&cb)
{
    funcs_.enqueue(std::move(cb));
    if (!isInLoopThread() || !looping_.load(std::memory_order_acquire))
    {
        wakeup();
    }
}

