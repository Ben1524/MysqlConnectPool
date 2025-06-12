/**
*@ClassName EventLoopThreadPool
*@Author cxk
*@Data 25-6-12 下午5:33
*/
//

#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H
#include <NonCopyable.h>
#include "event/EventLoopThread.h"

namespace cxk
{
class EventLoopThreadPool:public NonCopyable
{
public:
    EventLoopThreadPool()=delete;
    explicit EventLoopThreadPool(std::size_t threadNum,const std::string& name ="EventLoopThreadPool");
    void start();
    void wait();
    std::size_t size() const;
    EventLoop *getNextLoop();
    /**
     * @brief 获取指定索引的EventLoop
     * @param index 索引
     * @return 返回对应索引的EventLoop指针
     */
    EventLoop *getLoop(std::size_t index) const;
    /**
     * @brief 获取线程池名称
     * @return 返回线程池名称
     */
    const std::string &getName() const;

    std::vector<EventLoop *> getLoops() const;
private:
    std::vector<std::shared_ptr<EventLoopThread>> loopThreadVec_; ///< 存储线程池中的线程
    std::atomic<std::size_t> loopIndex_{0};
    std::string name_; ///< 线程池名称
};


}


#endif //EVENTLOOPTHREADPOOL_H
