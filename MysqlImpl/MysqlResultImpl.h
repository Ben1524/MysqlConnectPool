//
// Created by cxk_zjq on 25-6-2.
//

#ifndef MYSQLCONNECTPOOL_MYSQLRESULTIMPL_H
#define MYSQLCONNECTPOOL_MYSQLRESULTIMPL_H
#include <mysqlx/common.h>
#include <mysql_connection.h>
#include <mysqlx/devapi/result.h>
#include <mysqlx/xdevapi.h>
#include <db/ResultImpl.h>
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cxk
{
/*
 * @brief 实现基于mysqlx的非阻塞io的数据库接口，实现高并发的读取
 */
class MysqlResultImpl : public ResultImpl
{
public:
    MysqlResultImpl() = default;
    virtual ~MysqlResultImpl() = default;

    SizeType size() const noexcept override;
    RowSizeType columns() const noexcept override;
    const char *columnName(RowSizeType number) const override;
    SizeType affectedRows() const noexcept override;
    RowSizeType columnNumber(const char colName[]) const override;
    const char *getValue(SizeType row, RowSizeType column) const override;
    bool isNull(SizeType row, RowSizeType column) const override;
    FieldSizeType getLength(SizeType row, RowSizeType column) const override;
    unsigned long long insertId() const noexcept override;
private:
    const std::shared_ptr<mysqlx::Result> resultPtr_;
    Result::SizeType rowsNumber_{0};
    std::vector<mysqlx::Column> fieldArray_; // 列名
    Result::RowSizeType fieldsNumber_{0};
    const SizeType affectedRows_;  // 受影响的行数
    const unsigned long long insertId_;
    std::shared_ptr<std::unordered_map<std::string, RowSizeType>> fieldsMapPtr_;  // 字段名-->索引
    /* std::vector：每个元素代表一行数据。
     std::pair<char **, std::vector<unsigned long>>：
     char **：指向该行数据的字符指针数组（每个元素是字段值的起始地址）。
     std::vector<unsigned long>：存储每个字段值的长度（用于处理二进制数据或包含 NULL 的情况）。*/
    std::shared_ptr<std::vector<std::pair<char **, std::vector<unsigned long>>>> rowsPtr_;


};

} // cxk

#endif //MYSQLCONNECTPOOL_MYSQLRESULTIMPL_H
