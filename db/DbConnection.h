/**
 *
 *  @file DbConnection.h
 *  @author An Tao
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  https://github.com/an-tao/drogon
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Drogon
 *
 */
#ifndef DBCONNECTION_H
#define DBCONNECTION_H
#pragma once
// #include <DbClient.h>
#include <string_view>
#include <event/EventLoop.h>
#include <Result.h>
#include <NonCopyable.h>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <map>
#include <absl/log/absl_log.h>

namespace cxk
{
#if __cplusplus >= 201703L | defined _WIN32
using SharedMutex = std::shared_mutex;
#else
using SharedMutex = std::shared_timed_mutex;
#endif

using QueryCallback = std::function<void(const Result &)>;
using ExceptPtrCallback = std::function<void(const std::exception_ptr &)>;
using ResultCallback = std::function<void(const Result &)>;

enum class ConnectStatus
{
    None = 0,
    Connecting,
    SettingCharacterSet,
    Ok,
    Bad
};

struct SqlCmd
{
    std::string_view sql_;
    size_t parametersNumber_;
    std::vector<const char *> parameters_;
    std::vector<int> lengths_;
    std::vector<int> formats_;
    QueryCallback callback_;
    ExceptPtrCallback exceptionCallback_;
    std::string preparingStatement_;
    bool isChanging_{false};
    SqlCmd(std::string_view &&sql,
           size_t paraNum,
           std::vector<const char *> &&parameters,
           std::vector<int> &&length,
           std::vector<int> &&format,
           QueryCallback &&cb,
           ExceptPtrCallback &&exceptCb)
        : sql_(std::move(sql)),
          parametersNumber_(paraNum),
          parameters_(std::move(parameters)),
          lengths_(std::move(length)),
          formats_(std::move(format)),
          callback_(std::move(cb)),
          exceptionCallback_(std::move(exceptCb))
    {
    }
};

class DbConnection;
using DbConnectionPtr = std::shared_ptr<DbConnection>;

class DbConnection : public NonCopyable
{
  public:
    using DbConnectionCallback = std::function<void(const DbConnectionPtr &)>;

    explicit DbConnection(EventLoop *loop) : loop_(loop)
    {
        // 空回调实现
        exceptionCallback_ = [](const std::exception_ptr &)
        {
            ABSL_LOG(WARNING) << "DbConnection exception callback is not set!";
        };
        callback_ = [](const Result &)
        {
            ABSL_LOG(WARNING) << "DbConnection callback is not set!";
        };
        idleCb_ = []()
        {
            ABSL_LOG(WARNING) << "DbConnection idle callback is not set!";
        };
        okCallback_ = [](const DbConnectionPtr &)
        {
            ABSL_LOG(WARNING) << "DbConnection ok callback is not set!";
        };
        if (loop_ == nullptr)
        {
            std::cerr << "EventLoop is null!" << std::endl;
            throw std::runtime_error("EventLoop is null!");
        }
    }

    virtual void init(){};

    /**
     * @brief 设置操作成功回调函数
     *
     * 当数据库操作成功完成时，将调用此回调函数
     *
     * @param cb 成功回调函数，类型为DbConnectionCallback
     */
    void setOkCallback(const DbConnectionCallback &cb)
    {
        okCallback_ = cb;
    }


    /**
     * @brief 设置关闭连接回调函数
     *
     * 当数据库连接关闭时，将调用此回调函数
     *
     * @param cb 关闭回调函数，类型为DbConnectionCallback
     */
    void setCloseCallback(const DbConnectionCallback &cb)
    {
        closeCallback_ = cb;
    }

    /**
     * @brief 设置空闲状态回调函数
     *
     * 当数据库连接处于空闲状态时，将调用此回调函数
     *
     * @param cb 空闲状态回调函数，类型为std::function<void()>
     */
    void setIdleCallback(const std::function<void()> &cb)
    {
        idleCb_ = cb;
    }

    /**
     * @brief 执行SQL语句
     *
     * 异步执行SQL语句并通过回调返回结果
     *
     * @param sql 要执行的SQL语句
     * @param paraNum 参数数量
     * @param parameters SQL参数值数组
     * @param length 参数长度数组
     * @param format 参数格式数组
     * @param rcb 结果回调函数，用于处理查询结果
     * @param exceptCallback 异常回调函数，用于处理执行过程中发生的异常
     */
    virtual void execSql(
        std::string_view &&sql,
        size_t paraNum,
        std::vector<const char *> &&parameters,
        std::vector<int> &&length,
        std::vector<int> &&format,
        ResultCallback &&rcb,
        std::function<void(const std::exception_ptr &)> &&exceptCallback) = 0;

    /**
     * @brief 批量执行SQL命令
     *
     * 异步批量执行一系列SQL命令
     *
     * @param sqlCommands 要执行的SQL命令队列
     */
    virtual void batchSql(
        std::deque<std::shared_ptr<SqlCmd>> &&sqlCommands) = 0;


    virtual ~DbConnection()
    {
    }

    ConnectStatus status() const
    {
        return status_;
    }

    EventLoop *loop()
    {
        return loop_;
    }

    virtual void disconnect() = 0;

    bool isWorking() const
    {
        return isWorking_;
    }

  protected:
    QueryCallback callback_;
    EventLoop *loop_;
    std::function<void()> idleCb_;
    ConnectStatus status_{ConnectStatus::None};
    DbConnectionCallback closeCallback_{[](const DbConnectionPtr &) {}};
    DbConnectionCallback okCallback_{[](const DbConnectionPtr &) {}};
    std::function<void(const std::exception_ptr &)> exceptionCallback_;
    bool isWorking_{false};

    static std::map<std::string, std::string> parseConnString(
        const std::string &);

};

}  // namespace cxk

#endif