/**
*@ClassName EventDispatcher
*@Author cxk
*@Data 25-6-2 下午10:18
*/
//

#include <cassert>
#include "EventDispatcher.h"
#include "EventLoop.h"

void cxk::EventDispatcher::disableAll()
{
    events_ = getKNoneEvent();
    update();
}

cxk::EventDispatcher::EventDispatcher(cxk::EventLoop *loop, int fd)
: loop_(loop), fd_(fd), events_(0), realEvents_(0), index_(-1), tied_(false)
{

}

cxk::EventDispatcher::~EventDispatcher()
{

}

void cxk::EventDispatcher::setReadCallback(const cxk::EventDispatcher::EventCallback &cb)
{
    readCallback_ = cb;
}

void cxk::EventDispatcher::setWriteCallback(const cxk::EventDispatcher::EventCallback &cb)
{
    writeCallback_ = cb;
}

void cxk::EventDispatcher::setCloseCallback(const cxk::EventDispatcher::EventCallback &cb)
{
    closeCallback_ = cb;
}

void cxk::EventDispatcher::setErrorCallback(const cxk::EventDispatcher::EventCallback &cb)
{
    errorCallback_ = cb;
}

void cxk::EventDispatcher::setEventCallback(const cxk::EventDispatcher::EventCallback &cb)
{
    eventCallback_ = cb;
}

void cxk::EventDispatcher::setReadCallback(cxk::EventDispatcher::EventCallback &&cb)
{
    readCallback_ = std::move(cb);
}

void cxk::EventDispatcher::setWriteCallback(cxk::EventDispatcher::EventCallback &&cb)
{
    writeCallback_ = std::move(cb);
}

void cxk::EventDispatcher::setCloseCallback(cxk::EventDispatcher::EventCallback &&cb)
{
    closeCallback_ = std::move(cb);
}

void cxk::EventDispatcher::setErrorCallback(cxk::EventDispatcher::EventCallback &&cb)
{
    errorCallback_ = std::move(cb);
}

void cxk::EventDispatcher::setEventCallback(cxk::EventDispatcher::EventCallback &&cb)
{
    eventCallback_ = std::move(cb);
}

int cxk::EventDispatcher::getFd() const
{
    return fd_;
}

int cxk::EventDispatcher::getEvents() const
{
    return events_;
}

int cxk::EventDispatcher::getRealEvents() const
{
    return realEvents_;
}

bool cxk::EventDispatcher::isNoneEvent() const
{
     return events_ == getKNoneEvent();
}

const int cxk::EventDispatcher::getKNoneEvent()
{
    static const int kNoneEvent = 0;
    return kNoneEvent;
}

void cxk::EventDispatcher::update()
{
    loop_->updateChannel(this);
}

void cxk::EventDispatcher::remove()
{
    assert(events_ == getKNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

cxk::EventLoop *cxk::EventDispatcher::getLoop() const
{
    return loop_;
}

void cxk::EventDispatcher::tie(const std::shared_ptr<void> &obj)
{
    tied_ = true;
    tieObj_ = obj;
}

void cxk::EventDispatcher::enableReading()
{
    events_ |= getKReadEvent();
    update();
}
void cxk::EventDispatcher::enableWriting()
{
    events_ |= getKWriteEvent();
    update();
}

void cxk::EventDispatcher::disableReading()
{
    events_ &= ~getKReadEvent();
    update();
}
void cxk::EventDispatcher::disableWriting()
{
    events_ &= ~getKWriteEvent();
    update();
}



const int cxk::EventDispatcher::getKReadEvent()
{
    static const int kReadEvent = 1 << 0; // 读事件
    return kReadEvent;
}

const int cxk::EventDispatcher::getKWriteEvent()
{
    static const int kWriteEvent = 1 << 1; // 写事件
    return kWriteEvent;
}

bool cxk::EventDispatcher::isWriting() const
{
    return (events_ & getKWriteEvent()) != 0;
}

bool cxk::EventDispatcher::isReading() const
{
    return (events_ & getKReadEvent()) != 0;
}

void cxk::EventDispatcher::updateEvents(int events)
{
    events_ = events;
    update();
}

int cxk::EventDispatcher::setRealEvents(int revt)
{
    realEvents_=revt;
    return revt;
}

int cxk::EventDispatcher::getState() const
{
    return state_;
}
int cxk::EventDispatcher::setState(int index)
{
    state_ = index;
    return state_;
}



