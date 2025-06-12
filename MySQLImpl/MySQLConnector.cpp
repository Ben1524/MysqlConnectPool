/**
 *
 *  @file MySQLConnector.cc
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

#include "MySQLConnector.h"
#include "MySQLResultImpl.h"
#include <algorithm>
#include <exception>
#include <db/DbTypes.h>
#include <string_view>
#include <poll.h>
#include <regex>
#include <mariadb/errmsg.h>
#include "Exception.h"

using namespace cxk;

namespace cxk
{
Result makeResult(std::shared_ptr<MYSQL_RES> &&r = nullptr,
                  Result::SizeType affectedRows = 0,
                  unsigned long long insertId = 0)
{
    return Result{std::make_shared<MySQLResultImpl>(std::move(r),
                                                    affectedRows,
                                                    insertId)};
}


}  // namespace drogon

MySQLConnector::MySQLConnector(EventLoop *loop,
                                 const std::string &connInfo)
    : DbConnection(loop),
      mysqlPtr_(std::shared_ptr<MYSQL>(new MYSQL, [](MYSQL *p) {
          mysql_close(p);
          delete p;
      }))
{
    static MysqlEnv env;
    static thread_local MysqlThreadEnv threadEnv;
    mysql_init(mysqlPtr_.get());
    mysql_options(mysqlPtr_.get(), MYSQL_OPT_NONBLOCK, nullptr);
    mysql_options(mysqlPtr_.get(), MYSQL_OPT_RECONNECT, &reconnect_);
    // Get the key and value
    auto connParams = parseConnString(connInfo);
    for (auto const &kv : connParams)
    {
        auto key = kv.first;
        auto value = kv.second;

        std::transform(key.begin(),
                       key.end(),
                       key.begin(),
                       [](unsigned char c) { return tolower(c); });
        // ABSL_LOG(INFO) << key << "=" << value;
        if (key == "host")
        {
            host_ = value;
        }
        else if (key == "user")
        {
            user_ = value;
        }
        else if (key == "dbname")
        {
            // LOG_DEBUG << "database:[" << value << "]";
            dbname_ = value;
        }
        else if (key == "port")
        {
            port_ = value;
        }
        else if (key == "password")
        {
            passwd_ = value;
        }
        else if (key == "client_encoding")
        {
            characterSet_ = value;
        }
    }
}
void MySQLConnector::init()
{
    loop_->queueInLoop([this]() {
        MYSQL *ret;
        status_ = ConnectStatus::Connecting;
        ABSL_LOG(INFO) << "Connecting to MySQL server: "
                  << "host=" << host_ << ", user=" << user_
                  << ", dbname=" << dbname_ << ", port=" << port_;

        // 设置超时选项（使用正确的方法）
        unsigned int timeout = 10; // 10 seconds timeout
        mysql_options(mysqlPtr_.get(), MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
        mysql_options(mysqlPtr_.get(), MYSQL_OPT_READ_TIMEOUT, &timeout);
        mysql_options(mysqlPtr_.get(), MYSQL_OPT_WRITE_TIMEOUT, &timeout);

        // 只调用一次 mysql_real_connect_start()
        waitStatus_ = mysql_real_connect_start(&ret,
                                              mysqlPtr_.get(),
                                              host_.empty() ? nullptr : host_.c_str(),
                                              user_.empty() ? nullptr : user_.c_str(),
                                              passwd_.empty() ? nullptr : passwd_.c_str(),
                                              dbname_.empty() ? nullptr : dbname_.c_str(),
                                              port_.empty() ? 3306 : atol(port_.c_str()),
                                              nullptr,
                                              0);

        // 检查连接状态
        if (waitStatus_ == 0) {
            // 连接立即完成（通常不会发生，因为是非阻塞模式）
            int errorNo = mysql_errno(mysqlPtr_.get());
            if (errorNo) {
                ABSL_LOG(FATAL) << "Failed to connect to MySQL: Error("
                              << errorNo << ") \""
                              << mysql_error(mysqlPtr_.get()) << "\"";
                if (closeCallback_) {
                    auto thisPtr = shared_from_this();
                    closeCallback_(thisPtr);
                }
                return;
            }
        }

        // 获取套接字并检查
        auto fd = mysql_get_socket(mysqlPtr_.get());
        if (fd < 0) {
            ABSL_LOG(FATAL) << "Connection with MySQL could not be established";
            if (closeCallback_) {
                auto thisPtr = shared_from_this();
                closeCallback_(thisPtr);
            }
            return;
        } else {
            ABSL_LOG(INFO) << "MySQL connection in progress, fd: " << fd;
        }

        // 创建事件调度器
        eventDispatcherPtr_ = std::make_unique<EventDispatcher>(loop_, fd);
        eventDispatcherPtr_->setEventCallback([this]() { handleEvent(); });
        setEventDispatcher();
    });
}


void MySQLConnector::execSql(std::string_view&& sql, size_t paraNum, std::vector<const char*>&& parameters, std::vector<int>&& length,
    std::vector<int>&& format, ResultCallback&& rcb, std::function<void(const std::exception_ptr&)>&& exceptCallback)
{
    ABSL_LOG(INFO) << "Executing SQL: " << sql;
    if (loop_->isInLoopThread())
    {
        execSqlInLoop(std::move(sql),
                      paraNum,
                      std::move(parameters),
                      std::move(length),
                      std::move(format),
                      std::move(rcb),
                      std::move(exceptCallback));
    }
    else
    {
        auto thisPtr = shared_from_this();
        loop_->queueInLoop(
            [thisPtr,
             sql = std::move(sql),
             paraNum,
             parameters = std::move(parameters),
             length = std::move(length),
             format = std::move(format),
             rcb = std::move(rcb),
             exceptCallback = std::move(exceptCallback)]() mutable {
                thisPtr->execSqlInLoop(std::move(sql),
                                       paraNum,
                                       std::move(parameters),
                                       std::move(length),
                                       std::move(format),
                                       std::move(rcb),
                                       std::move(exceptCallback));
            });
    }
}

void MySQLConnector::batchSql(std::deque<std::shared_ptr<SqlCmd>>&&)
{
    ABSL_LOG(FATAL) << "The mysql library does not support batch mode";
    exit(1);
}

void MySQLConnector::setEventDispatcher()
{
    ABSL_LOG(INFO) << "Setting event dispatcher for MySQL connection";
    if ((waitStatus_ & MYSQL_WAIT_READ) || (waitStatus_ & MYSQL_WAIT_EXCEPT))
    {
        ABSL_LOG(INFO) << "Enabling reading for MySQL connection";
        if (!eventDispatcherPtr_->isReading())
            eventDispatcherPtr_->enableReading();
    }
    if (waitStatus_ & MYSQL_WAIT_WRITE)
    {
        ABSL_LOG(INFO) << "Enabling writing for MySQL connection";
        if (!eventDispatcherPtr_->isWriting())
            eventDispatcherPtr_->enableWriting();
    }
    else
    {
        ABSL_LOG(INFO) << "Disabling writing for MySQL connection";
        if (eventDispatcherPtr_->isWriting())
            eventDispatcherPtr_->disableWriting();
    }
    if (waitStatus_ & MYSQL_WAIT_TIMEOUT)
    {
        ABSL_LOG(INFO) << "Setting timeout for MySQL connection";
        auto timeout = mysql_get_timeout_value(mysqlPtr_.get());
        auto thisPtr = shared_from_this();
        loop_->runAfter(timeout, [thisPtr]() { thisPtr->handleTimeout(); });
    }
}

void MySQLConnector::handleClosed()
{
    loop_->assertInLoopThread();
    if (status_ == ConnectStatus::Bad)
        return;
    status_ = ConnectStatus::Bad;
    eventDispatcherPtr_->disableAll();
    eventDispatcherPtr_->remove();
    assert(closeCallback_);
    auto thisPtr = shared_from_this();
    closeCallback_(thisPtr);
}

void MySQLConnector::disconnect()
{
    auto thisPtr = shared_from_this();
    std::promise<int> pro;
    auto f = pro.get_future();
    loop_->runInLoop([thisPtr, &pro]() {
        thisPtr->status_ = ConnectStatus::Bad;
        thisPtr->eventDispatcherPtr_->disableAll();
        thisPtr->eventDispatcherPtr_->remove();
        thisPtr->mysqlPtr_.reset();
        pro.set_value(1);
    });
    f.get();
}

void MySQLConnector::handleTimeout()
{
    int status = 0;
    status |= MYSQL_WAIT_TIMEOUT;
    MYSQL *ret;
    if (status_ == ConnectStatus::Connecting)
    {
        waitStatus_ = mysql_real_connect_cont(&ret, mysqlPtr_.get(), status);
        if (waitStatus_ == 0)
        {
            auto errorNo = mysql_errno(mysqlPtr_.get());
            if (!ret && errorNo)
            {
                ABSL_LOG(FATAL) << "Error(" << errorNo << ") \""
                          << mysql_error(mysqlPtr_.get()) << "\"";
                ABSL_LOG(FATAL) << "Failed to mysql_real_connect()";
                handleClosed();
                return;
            }
            // I don't think the programe can run to here.
            if (characterSet_.empty())
            {
                status_ = ConnectStatus::Ok;
                if (okCallback_)
                {
                    auto thisPtr = shared_from_this();
                    okCallback_(thisPtr);
                }
            }
            else
            {
                startSetCharacterSet();
                return;
            }
        }
        setEventDispatcher();
    }
    else if (status_ == ConnectStatus::SettingCharacterSet)
    {
        continueSetCharacterSet(status);
    }
    else if (status_ == ConnectStatus::Ok)
    {
    }
}

void MySQLConnector::handleCmd(int status)
{
    switch (execStatus_)
    {
        case ExecStatus::RealQuery:
        {
            int err = 0;
            waitStatus_ = mysql_real_query_cont(&err, mysqlPtr_.get(), status);
            ABSL_LOG(INFO) << "real_query:" << waitStatus_;
            if (waitStatus_ == 0)
            {
                if (err)
                {
                    execStatus_ = ExecStatus::None;
                    ABSL_LOG(FATAL) << "error:" << err << " status:" << status;
                    outputError();
                    return;
                }
                startStoreResult(false);
            }
            setEventDispatcher();
            break;
        }
        case ExecStatus::StoreResult:
        {
            MYSQL_RES *ret;
            waitStatus_ =
                mysql_store_result_cont(&ret, mysqlPtr_.get(), status);
            ABSL_LOG(INFO) << "store_result:" << waitStatus_;
            if (waitStatus_ == 0)
            {
                if (!ret && mysql_errno(mysqlPtr_.get()))
                {
                    execStatus_ = ExecStatus::None;
                    ABSL_LOG(FATAL) << "error";
                    outputError();
                    return;
                }
                getResult(ret);
            }
            setEventDispatcher();
            break;
        }
        case ExecStatus::NextResult:
        {
            int err;
            waitStatus_ = mysql_next_result_cont(&err, mysqlPtr_.get(), status);
            if (waitStatus_ == 0)
            {
                if (err)
                {
                    execStatus_ = ExecStatus::None;
                    ABSL_LOG(FATAL) << "error:" << err << " status:" << status;
                    outputError();
                    return;
                }
                startStoreResult(false);
            }
            setEventDispatcher();
            break;
        }
        case ExecStatus::None:
        {
            // Connection closed!
            if (waitStatus_ == 0)
                handleClosed();
            break;
        }
        default:
            return;
    }
}

void MySQLConnector::handleEvent()
{
    int status = 0;
    auto revents = eventDispatcherPtr_->getRealEvents();
    if (revents & POLLIN)
        status |= MYSQL_WAIT_READ;
    if (revents & POLLOUT)
        status |= MYSQL_WAIT_WRITE;
    if (revents & POLLPRI)
        status |= MYSQL_WAIT_EXCEPT;
    status = (status & waitStatus_);
    MYSQL *ret;
    if (status_ == ConnectStatus::Connecting)
    {
        waitStatus_ = mysql_real_connect_cont(&ret, mysqlPtr_.get(), status);
        if (waitStatus_ == 0)
        {
            auto errorNo = mysql_errno(mysqlPtr_.get());
            if (!ret && errorNo)
            {
                ABSL_LOG(FATAL) << "Error(" << errorNo << ") \""
                          << mysql_error(mysqlPtr_.get()) << "\"";
                ABSL_LOG(FATAL) << "Failed to mysql_real_connect()";
                handleClosed();
                return;
            }
            if (characterSet_.empty())
            {
                status_ = ConnectStatus::Ok;
                if (okCallback_)
                {
                    auto thisPtr = shared_from_this();
                    okCallback_(thisPtr);
                }
            }
            else
            {
                startSetCharacterSet();
                return;
            }
        }
        setEventDispatcher();
    }
    else if (status_ == ConnectStatus::Ok)
    {
        handleCmd(status);
    }
    else if (status_ == ConnectStatus::SettingCharacterSet)
    {
        continueSetCharacterSet(status);
    }
}

void MySQLConnector::continueSetCharacterSet(int status)
{
    int err;
    waitStatus_ = mysql_set_character_set_cont(&err, mysqlPtr_.get(), status);
    if (waitStatus_ == 0)
    {
        if (err)
        {
            ABSL_LOG(FATAL) << "Error(" << err << ") \""
                      << mysql_error(mysqlPtr_.get()) << "\"";
            ABSL_LOG(FATAL) << "Failed to mysql_set_character_set_cont()";
            handleClosed();
            return;
        }
        status_ = ConnectStatus::Ok;
        if (okCallback_)
        {
            auto thisPtr = shared_from_this();
            okCallback_(thisPtr);
        }
    }
    setEventDispatcher();
}

void MySQLConnector::startSetCharacterSet()
{
    int err;
    waitStatus_ = mysql_set_character_set_start(&err,
                                                mysqlPtr_.get(),
                                                characterSet_.data());
    if (waitStatus_ == 0)
    {
        if (err)
        {
            ABSL_LOG(FATAL) << "Error(" << err << ") \""
                      << mysql_error(mysqlPtr_.get()) << "\"";
            ABSL_LOG(FATAL) << "Failed to mysql_set_character_set_start()";
            handleClosed();
            return;
        }
        status_ = ConnectStatus::Ok;
        if (okCallback_)
        {
            auto thisPtr = shared_from_this();
            okCallback_(thisPtr);
        }
    }
    else
    {
        status_ = ConnectStatus::SettingCharacterSet;
    }
    setEventDispatcher();
}

void MySQLConnector::execSqlInLoop(
    std::string_view &&sql,
    size_t paraNum,
    std::vector<const char *> &&parameters,
    std::vector<int> &&length,
    std::vector<int> &&format,
    ResultCallback &&rcb,
    std::function<void(const std::exception_ptr &)> &&exceptCallback)
{
    ABSL_LOG(INFO) << sql;
    assert(paraNum == parameters.size());
    assert(paraNum == length.size());
    assert(paraNum == format.size());
    assert(rcb);
    assert(!isWorking_);
    assert(!sql.empty());

    callback_ = std::move(rcb);
    isWorking_ = true;
    exceptionCallback_ = std::move(exceptCallback);
    sql_.clear();
    if (paraNum > 0)
    {
        std::string::size_type pos = 0;
        std::string::size_type seekPos = std::string::npos;
        for (size_t i = 0; i < paraNum; ++i)
        {
            seekPos = sql.find('?', pos);
            if (seekPos == std::string::npos)
            {
                auto sub = sql.substr(pos);
                sql_.append(sub.data(), sub.length());
                pos = seekPos;
                break;
            }
            else
            {
                auto sub = sql.substr(pos, seekPos - pos);
                sql_.append(sub.data(), sub.length());
                pos = seekPos + 1;
                switch (format[i])
                {
                case cxk::type::MySqlTiny:
                        sql_.append(std::to_string(*((char*)parameters[i])));
                        break;
                    case cxk::type::MySqlShort:
                        sql_.append(std::to_string(*((short *)parameters[i])));
                        break;
                    case cxk::type::MySqlLong:
                        sql_.append(
                            std::to_string(*((int32_t *)parameters[i])));
                        break;
                    case cxk::type::MySqlLongLong:
                        sql_.append(
                            std::to_string(*((int64_t *)parameters[i])));
                        break;
                    case cxk::type::MySqlNull:
                        sql_.append("NULL");
                        break;
                    case cxk::type::MySqlString:
                    {
                        sql_.append("'");
                        std::string to(length[i] * 2, '\0');
                        auto len = mysql_real_escape_string(mysqlPtr_.get(),
                                                            (char *)to.c_str(),
                                                            parameters[i],
                                                            length[i]);
                        to.resize(len);
                        sql_.append(to);
                        sql_.append("'");
                    }
                    break;
                    case cxk::type::DrogonDefaultValue:
                        sql_.append("default");
                        break;
                    default:
                        ABSL_LOG(FATAL)
                            << "MySQL does not recognize the parameter type";
                        abort();
                        break;
                }
            }
        }
        if (pos < sql.length())
        {
            auto sub = sql.substr(pos);
            sql_.append(sub.data(), sub.length());
        }
    }
    else
    {
        sql_ = std::string(sql.data(), sql.length());
    }
    ABSL_LOG(INFO) << "Prepared SQL: " << sql_;
    startQuery();
    setEventDispatcher();
}

void MySQLConnector::outputError()
{
    eventDispatcherPtr_->disableAll();
    auto errorNo = mysql_errno(mysqlPtr_.get());
    ABSL_LOG(FATAL) << "Error(" << errorNo << ") [" << mysql_sqlstate(mysqlPtr_.get())
              << "] \"" << mysql_error(mysqlPtr_.get()) << "\"";
    ABSL_LOG(FATAL) << "sql:" << sql_;
    if (isWorking_)
    {
        // TODO: exception type
        auto exceptPtr = std::make_exception_ptr(
            SqlError(mysql_error(mysqlPtr_.get()), sql_));
        exceptionCallback_(exceptPtr);
        exceptionCallback_ = nullptr;

        callback_ = nullptr;
        isWorking_ = false;
        if (errorNo != CR_SERVER_GONE_ERROR && errorNo != CR_SERVER_LOST)
        {
            idleCb_();
        }
    }
    if (errorNo == CR_SERVER_GONE_ERROR || errorNo == CR_SERVER_LOST)
    {
        handleClosed();
    }
}

void MySQLConnector::startQuery()
{
    // return ;
    int err;
    // int mysql_real_query_start(int *ret, MYSQL *mysql, const char *q,
    // unsigned long length)
    waitStatus_ = mysql_real_query_start(&err,
                                         mysqlPtr_.get(),
                                         sql_.c_str(),
                                         sql_.length());
    ABSL_LOG(INFO) << "real_query:" << waitStatus_;
    execStatus_ = ExecStatus::RealQuery;
    if (waitStatus_ == 0)
    {
        if (err)
        {
            ABSL_LOG(FATAL) << "error";
            loop_->queueInLoop(
                [thisPtr = shared_from_this()] { thisPtr->outputError(); });
            return;
        }
        startStoreResult(true);
    }
}

void MySQLConnector::startStoreResult(bool queueInLoop)
{
    MYSQL_RES *ret;
    execStatus_ = ExecStatus::StoreResult;
    waitStatus_ = mysql_store_result_start(&ret, mysqlPtr_.get());
    ABSL_LOG(INFO) << "store_result:" << waitStatus_;
    if (waitStatus_ == 0)
    {
        execStatus_ = ExecStatus::None;
        if (!ret && mysql_errno(mysqlPtr_.get()))
        {
            if (queueInLoop)
            {
                loop_->queueInLoop(
                    [thisPtr = shared_from_this()] { thisPtr->outputError(); });
            }
            else
            {
                outputError();
            }
            return;
        }
        if (queueInLoop)
        {
            loop_->queueInLoop([thisPtr = shared_from_this(), ret] {
                thisPtr->getResult(ret);
            });
        }
        else
        {
            getResult(ret);
        }
    }
}

void MySQLConnector::getResult(MYSQL_RES *res)
{
    auto resultPtr = std::shared_ptr<MYSQL_RES>(res, [](MYSQL_RES *r) {
        mysql_free_result(r);
    });
    auto Result = makeResult(std::move(resultPtr),
                             mysql_affected_rows(mysqlPtr_.get()),
                             mysql_insert_id(mysqlPtr_.get()));
    if (isWorking_)
    {
        callback_(Result);
        if (!mysql_more_results(mysqlPtr_.get()))
        {
            callback_ = nullptr;
            exceptionCallback_ = nullptr;
            isWorking_ = false;
            idleCb_();
        }
        else
        {
            execStatus_ = ExecStatus::NextResult;
            int err;
            waitStatus_ = mysql_next_result_start(&err, mysqlPtr_.get());
            if (waitStatus_ == 0)
            {
                if (err)
                {
                    execStatus_ = ExecStatus::None;
                    ABSL_LOG(FATAL) << "error:" << err;
                    outputError();
                    return;
                }
                startStoreResult(false);
            }
        }
    }
}
