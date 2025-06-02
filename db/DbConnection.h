//
// Created by cxk_zjq on 25-6-2.
//

#ifndef MYSQLCONNECTPOOL_DBCONNECTION_H
#define MYSQLCONNECTPOOL_DBCONNECTION_H

#include <drogon/config.h>
#include <drogon/orm/DbClient.h>
#include <trantor/net/EventLoop.h>

#include <string_view>
#include <NonCopyable.h>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>


namespace cxk
{


/**
 * @brief 数据库连接状态枚举
 */
enum class ConnectStatus
{
    None = 0,
    Connecting,
    SettingCharacterSet,
    Ok,
    Bad
};


class DbConnection;
using DbConnectionPtr = std::shared_ptr<DbConnection>;

class DbConnection : public NonCopyable
{

};

}
#endif //MYSQLCONNECTPOOL_DBCONNECTION_H
