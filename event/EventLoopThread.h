/**
*@ClassName EventLoopThread
*@Author cxk
*@Data 25-6-12 下午4:28
*/
//

#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H
#include <future>
#include <memory>
#include <absl/synchronization/mutex.h>
#include <string>
#include <NonCopyable.h>
#include <event/EventLoop.h>
namespace cxk
{
class EventLoopThread :public NonCopyable
{
public:
    explicit EventLoopThread(const std::string& threadName="EventLoop");

    ~EventLoopThread();

    void wait();

    EventLoop* getLoop();
    void run();

private:
    std::shared_ptr<EventLoop> loop_;
    absl::Mutex loopMutex_;

    std::string loopThreadName_;
    void loopFuncs();
    std::promise<std::shared_ptr<EventLoop>> promiseForLoopPointer_;
    std::promise<int> promiseForRun_;
    std::promise<int> promiseForLoop_;
    std::once_flag once_;
    std::thread thread_;
};


}

#endif //EVENTLOOPTHREAD_H
