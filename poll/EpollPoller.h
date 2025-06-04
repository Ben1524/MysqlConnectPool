/**
*@ClassName EpollPoller
*@Author cxk
*@Data 25-6-4 上午11:08
*/
//

#ifndef MYSQLCONNECTPOOL_EPOLLPOLLER_H
#define MYSQLCONNECTPOOL_EPOLLPOLLER_H
#include "NonCopyable.h"
#include "event/EventLoop.h"
#include <memory>
#include <map>
using EventList = std::vector<struct epoll_event>;
namespace cxk
{
class EventDispatcher; ///< 前向声明EventDispatcher类
/**
 * @brief Poller类是事件循环的抽象基类，负责处理事件的轮询和分发。
 */
class EpollPoller: public NonCopyable
{
public:
    explicit EpollPoller(EventLoop *loop);
    virtual ~EpollPoller();
    void assertInLoopThread();
    void poll(int timeoutMs, EventDispatcherList *activeChannels);
    void registerEventDispatcher(EventDispatcher *channel);
    void removeEventDispatcher(EventDispatcher *channel);
    void resetAfterFork();
    static EpollPoller * newPoller(EventLoop *loop);


private:
    static const int kInitEventListSize = 16;
    int epollfd_;

    EventList events_;
    bool updateEventDispatcher(int operation, EventDispatcher *channel);
    void fillActiveDispatchers(int numEvents,
                            EventDispatcherList *activeChannels) const;
    using EventDispatcherMap = std::map<int, EventDispatcher *>;
    EventDispatcherMap dispatchers_; ///< 管理所有的事件分发器
    EventLoop *loop_; ///< 事件循环对象指针

};

} // cxk

#endif //MYSQLCONNECTPOOL_EPOLLPOLLER_H
