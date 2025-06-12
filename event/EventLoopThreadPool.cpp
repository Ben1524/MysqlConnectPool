

#include "EventLoopThreadPool.h"
using namespace cxk;

cxk::EventLoopThreadPool::EventLoopThreadPool(size_t threadNum,
                                              const std::string &name)
    : loopIndex_(0),name_(name)
{
    for (size_t i = 0; i < threadNum; ++i)
    {
        loopThreadVec_.emplace_back(std::make_shared<EventLoopThread>(name));
    }
}
void EventLoopThreadPool::start()
{
    for (unsigned int i = 0; i < loopThreadVec_.size(); ++i)
    {
        loopThreadVec_[i]->run();
    }
}

void EventLoopThreadPool::wait()
{
    for (unsigned int i = 0; i < loopThreadVec_.size(); ++i)
    {
        loopThreadVec_[i]->wait();
    }
}

std::size_t EventLoopThreadPool::size() const
{
    return loopThreadVec_.size();
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    if (loopThreadVec_.size() > 0)
    {
        size_t index = loopIndex_.fetch_add(1, std::memory_order_relaxed);
        EventLoop *loop =
            loopThreadVec_[index % loopThreadVec_.size()]->getLoop();
        return loop;
    }
    return nullptr;
}
EventLoop *EventLoopThreadPool::getLoop(size_t id) const
{
    if (id < loopThreadVec_.size())
        return loopThreadVec_[id]->getLoop();
    return nullptr;
}

const std::string& EventLoopThreadPool::getName() const
{
    return name_;
}

std::vector<EventLoop *> EventLoopThreadPool::getLoops() const
{
    std::vector<EventLoop *> ret;
    for (auto &loopThread : loopThreadVec_)
    {
        ret.push_back(loopThread->getLoop());
    }
    return ret;
}
