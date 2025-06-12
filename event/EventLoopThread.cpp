/**
*@ClassName EventLoopThread
*@Author cxk
*@Data 25-6-12 下午4:28
*/
//

#include "EventLoopThread.h"
#include <sys/prctl.h>
using namespace cxk;

EventLoopThread::EventLoopThread(const std::string &threadName)
    : loop_(nullptr),
      loopThreadName_(threadName),
      thread_([this]() { loopFuncs(); })
{
    auto f = promiseForLoopPointer_.get_future(); // 确保loop先初始化完成。
    loop_ = f.get();
}

void EventLoopThread::loopFuncs()
{
    ::prctl(PR_SET_NAME,loopThreadName_.c_str());
    thread_local static std::shared_ptr<EventLoop> loopPtr=
        std::make_shared<EventLoop>();
    assert(loopPtr->threadId() == std::this_thread::get_id());
    loopPtr->queueInLoop([this]() { promiseForLoop_.set_value(1); });
    promiseForLoopPointer_.set_value(loopPtr);

    auto f = promiseForRun_.get_future();
    (void)f.get(); // 同步语义
    loopPtr->loop();
    {
        absl::MutexLock lock(&loopMutex_);
        loop_=nullptr;
    }
}

void EventLoopThread::run()   // 启动事件循环线程，确保发生在loopPtr->loop();之前
{
    std::call_once(once_, [this]() {
        auto f = promiseForLoop_.get_future();
        promiseForRun_.set_value(1);
        // Make sure the event loop loops before returning.
        (void)f.get();
    });
}
EventLoopThread::~EventLoopThread()
{
    run();
    std::shared_ptr<EventLoop> loop;
    {
        absl::MutexLock lock(&loopMutex_);
        loop = loop_;
    }
    if (loop)
    {
        loop->quit();
    }
    if (thread_.joinable())
    {
        thread_.join();
    }
}

void EventLoopThread::wait()
{
    thread_.join();
}
