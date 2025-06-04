/**
*@ClassName EpollPoller
*@Author cxk
*@Data 25-6-4 上午11:08
*/
//

#include "EpollPoller.h"
#include "EpollPoller.h"
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <iostream>
#include <cstring>
#include <spdlog/spdlog.h>
#include "event/EventDispatcher.h"
namespace cxk
{
#if defined __linux__
static_assert(EPOLLIN == POLLIN, "EPOLLIN != POLLIN");
static_assert(EPOLLPRI == POLLPRI, "EPOLLPRI != POLLPRI");
static_assert(EPOLLOUT == POLLOUT, "EPOLLOUT != POLLOUT");
static_assert(EPOLLRDHUP == POLLRDHUP, "EPOLLRDHUP != POLLRDHUP");
static_assert(EPOLLERR == POLLERR, "EPOLLERR != POLLERR");
static_assert(EPOLLHUP == POLLHUP, "EPOLLHUP != POLLHUP");
#endif

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}  // namespace

EpollPoller::EpollPoller(EventLoop *loop):
    loop_(loop),epollfd_(::epoll_create1(EPOLL_CLOEXEC)),events_(kInitEventListSize)
{
}

EpollPoller::~EpollPoller()
{
    close(epollfd_);
}
void EpollPoller::assertInLoopThread()
{
    if (this->loop_){
        loop_->assertInLoopThread();
    }
}

void EpollPoller::resetAfterFork()
{

}

EpollPoller * EpollPoller::newPoller(EventLoop *loop)
{
    return new EpollPoller(loop);
}

void EpollPoller::poll(int timeoutMs, EventDispatcherList *activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(),  // 事件起始数组
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;
    if (numEvents>0)
    {
        fillActiveDispatchers(numEvents,activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size())  // 如果可以做到全部触发，说明此时的处理事件很多，尝试扩容适应
        {
            events_.resize(events_.size() * 2);
        }
    }else if (numEvents == 0)
    {
    }
    else
    {
        // error happens, log uncommon ones
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            spdlog::error("EpollPoller::poll() error: {}", strerror(savedErrno));
        }
    }
    return;
}

void EpollPoller::fillActiveDispatchers(int numEvents, EventDispatcherList *activeDispathcer) const
{
    // 判断事件数量是不是超过限制了
    assert(static_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i)
    {
        EventDispatcher *dispatcher=static_cast<EventDispatcher*>(events_[i].data.ptr);
        int fd = dispatcher->getFd();
        auto it = dispatchers_.find(fd); // 返回值是
        assert(it != dispatchers_.end());
        assert(it->second==dispatcher);
        dispatcher->setRealEvents(events_[i].events);
        activeDispathcer->push_back(dispatcher);
    }
}

bool EpollPoller::updateEventDispatcher(int operation, EventDispatcher *channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events=channel->getEvents();
    event.data.ptr=channel;
    int fd = channel->getFd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        spdlog::error("EpollPoller::registerEventDispatcher() epoll_ctl error: {}, fd: {}", strerror(errno), fd);
        return false;
    }
    return true;
}

void EpollPoller::registerEventDispatcher(EventDispatcher *channel)
{
    assertInLoopThread();
    assert(channel->getFd() >= 0);
    const int index = channel->getState();
    if (index == kNew || index == kDeleted)
    {
        channel->setState(kAdded); // 设置为已添加状态
        updateEventDispatcher(EPOLL_CTL_ADD, channel);
    }
    else
    {
        assert(index == kAdded); // 已经添加过了
        if (channel->isNoneEvent()) 
        {
            updateEventDispatcher(EPOLL_CTL_DEL, channel);
            channel->setState(kDeleted);
        }
        else
        {
            updateEventDispatcher(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeEventDispatcher(EventDispatcher *channel)
{
    EpollPoller::assertInLoopThread();
    assert(channel->isNoneEvent()); // 如果有事件
    int index = channel->getState();
    assert(index == kAdded || index == kDeleted);
    if (index == kAdded)
    {
        updateEventDispatcher(EPOLL_CTL_DEL, channel);
    }
    channel->setState(kNew);   // 标记为新的，没有被添加的
}


} // cxk