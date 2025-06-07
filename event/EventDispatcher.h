/**
 * @ClassName EventDispatcher
 * @Author cxk
 * @Data 25-6-2 下午10:18
 * @Description 事件分发器，负责将底层I/O事件映射到用户注册的回调函数
 *              实现了Reactor模式中的事件处理器角色
 */

#ifndef MYSQLCONNECTPOOL_EVENTDISPATCHER_H
#define MYSQLCONNECTPOOL_EVENTDISPATCHER_H
#include "NonCopyable.h"
#include <functional>
#include <memory>

#include "time/TimerQueue.h"

namespace cxk
{
class EventLoop; ///< 前向声明EventLoop类

/**
 * @brief 事件分发器，负责将底层I/O事件（如可读、可写）映射到用户注册的回调函数
 *
 * 该类是Reactor模式的核心组件，封装了对单个文件描述符(fd)的事件监听和处理
 * 每个EventDispatcher实例关联一个fd，并负责将该fd上发生的事件通知给用户
 * 采用观察者模式，当EventLoop发生变化时自动通知
 */
class EventDispatcher:public NonCopyable
{
public:
    friend class EventLoop;
    friend class EpollPoller;
    friend class KQueue;
    friend class PollPoller;

    using EventCallback = std::function<void()>; ///< 事件回调函数类型

    /**
     * @brief 构造函数，创建一个事件分发器实例
     *
     * @param loop 所属的EventLoop事件循环
     * @param fd 需要监听的文件描述符
     */
    explicit EventDispatcher(EventLoop *loop, int fd);

    /**
     * @brief 析构函数
     */
    ~EventDispatcher();

    /**
     * @brief 设置读事件回调函数
     *
     * @param cb 读事件发生时调用的回调函数
     */
    void setReadCallback(const EventCallback &cb);

    /**
     * @brief 设置写事件回调函数
     *
     * @param cb 写事件发生时调用的回调函数
     */
    void setWriteCallback(const EventCallback &cb);

    /**
     * @brief 设置错误事件回调函数
     *
     * @param cb 错误事件发生时调用的回调函数
     */
    void setErrorCallback(const EventCallback &cb);

    /**
     * @brief 设置关闭事件回调函数
     *
     * @param cb 关闭事件发生时调用的回调函数
     */
    void setCloseCallback(const EventCallback &cb);

    /**
     * @brief 设置通用事件回调函数
     *
     * @param cb 事件发生时调用的通用回调函数
     * @note 如果设置了通用回调函数，则其他特定事件回调函数将不会被调用
     */
    void setEventCallback(const EventCallback &cb);

    // 移动语义版本的回调函数设置
    void setReadCallback(EventCallback &&cb);
    void setWriteCallback(EventCallback &&cb);
    void setErrorCallback(EventCallback &&cb);
    void setCloseCallback(EventCallback &&cb);
    void setEventCallback(EventCallback &&cb);

    /**
     * @brief 获取关联的文件描述符
     *
     * @return int 文件描述符
     */
    int getFd() const;

    /**
     * @brief 获取当前注册的事件类型
     *
     * @return int 事件类型掩码
     */
    int getEvents() const;

    /**
     * @brief 获取当前实际发生的事件类型
     *
     * @return int 事件类型掩码
     */
    int getRealEvents() const;
    int setRealEvents(int revt);


    int getState()const;
    int setState(int index);

    /**
     * @brief 检查是否没有注册任何事件
     *
     * @return true 没有注册任何事件
     * @return false 至少注册了一个事件
     */
    bool isNoneEvent() const;

    /**
     * @brief 禁用所有事件监听
     */
    void disableAll();

    /**
     * @brief 从EventLoop中移除该事件分发器
     */
    void remove();

    /**
     * @brief 获取所属的EventLoop
     *
     * @return EventLoop* EventLoop指针
     */
    EventLoop* getLoop() const;

    /**
     * @brief 启用读事件监听
     */
    void enableReading();

    /**
     * @brief 禁用读事件监听
     */
    void disableReading();

    /**
     * @brief 启用写事件监听
     */
    void enableWriting();

    /**
     * @brief 禁用写事件监听
     */
    void disableWriting();

    /**
     * @brief 检查是否正在监听写事件
     *
     * @return true 正在监听写事件
     * @return false 未监听写事件
     */
    bool isWriting() const;

    /**
     * @brief 检查是否正在监听读事件
     *
     * @return true 正在监听读事件
     * @return false 未监听读事件
     */
    bool isReading() const;

    /**
     * @brief 更新事件分发器状态到Poller中
     *
     * 当注册的事件发生变化时，需要调用此方法通知Poller
     */
    void update();

    /**
     * @brief 直接更新事件类型
     *
     * @param events 新的事件类型掩码
     */
    void updateEvents(int events);

    /**
     * @brief 设置关联对象，用于确保回调安全
     *
     * @param obj 共享指针，通常是拥有该事件分发器的对象
     * @note 使用弱引用避免循环引用
     */
    void tie(const std::shared_ptr<void> &obj);


    static const int getKNoneEvent(); ///< 获取无事件类型常量
    static const int getKReadEvent(); ///< 获取可读事件类型常量
    static const int getKWriteEvent(); ///< 获取可写事件类型常量

private:
    EventLoop* loop_; ///< 所属的EventLoop
    const int fd_; ///< 监听的文件描述符
    int events_; ///< 当前注册的事件类型（如可读、可写）
    int realEvents_; ///< 实际发生的事件类型
    int state_; ///< 在Poller中状态
    bool addedToLoop_{false}; ///< 是否已添加到EventLoop中
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
    EventCallback eventCallback_; ///< 用户自定义事件回调函数
    std::weak_ptr<void> tie_;
    bool tied_;
};
}

#endif //MYSQLCONNECTPOOL_EVENTDISPATCHER_H