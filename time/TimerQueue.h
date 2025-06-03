/**
*@ClassName TimerQueue
*@Author cxk
*@Data 25-6-2 下午11:25
*/
//

#ifndef MYSQLCONNECTPOOL_TIMERQUEUE_H
#define MYSQLCONNECTPOOL_TIMERQUEUE_H
#include "Timer.h"
#include <memory>
#include <ctime>
#include <atomic>
#include <unordered_set>
#include <functional>
namespace cxk
{
class EventLoop;
class EventDispatcher;

using TimerPtr = std::shared_ptr<Timer>;

/**
 * @brief 定时器指针比较器，用于优先队列排序
 * 按照定时器触发时间倒序排列（最早触发的定时器位于队顶）
 */
struct TimerPtrComparer
{
    bool operator()(const TimerPtr &x, const TimerPtr &y) const
    {
        return *x > *y; // 利用Timer的operator>实现倒序排序
    }
};
class TimerQueue:public NonCopyable
{

};

}



#endif //MYSQLCONNECTPOOL_TIMERQUEUE_H
