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
#include <mariadb/mysql.h>
namespace cxk
{


class MySQLResultImpl : public ResultImpl
{
  public:
    MySQLResultImpl(std::shared_ptr<MYSQL_RES> r,
                    SizeType affectedRows,
                    unsigned long long insertId) noexcept;


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
    const std::shared_ptr<MYSQL_RES> result_;
    const Result::SizeType rowsNumber_;
    const MYSQL_FIELD *fieldArray_;
    const Result::RowSizeType fieldsNumber_;
    const SizeType affectedRows_;
    const unsigned long long insertId_;
    std::shared_ptr<std::unordered_map<std::string, RowSizeType>> fieldsMapPtr_;
    std::shared_ptr<std::vector<std::pair<char **, std::vector<unsigned long>>>>
        rowsPtr_;
};

} // cxk

#endif //MYSQLCONNECTPOOL_MYSQLRESULTIMPL_H
