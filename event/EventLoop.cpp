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

using namespace cxk;
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

}

EventLoop::EventLoop():
        looping_(false),
        threadId_(std::this_thread::get_id()),
        quit_(false),
        poller_(EpollPoller::newPoller(this)),
        currentActiveDispatcher_(nullptr),
        eventHandling_(false),
        timerQueue_(new TimerQueue(this))
{

}

void EventLoop::assertInLoopThread()
{
    if (!isInLoopThread())
    {
        abortNotInLoopThread();
    }
}
