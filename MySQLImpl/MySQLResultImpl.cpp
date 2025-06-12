/**
 *
 *  MySQLResultImpl.cc
 *  An Tao
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  https://github.com/an-tao/drogon
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Drogon
 *
 */

#include "MySQLResultImpl.h"
#include <algorithm>
#include <cassert>
#include <db/Exception.h>

using namespace cxk;

MySQLResultImpl::MySQLResultImpl(std::shared_ptr<MYSQL_RES> r, SizeType affectedRows, unsigned long long insertId) noexcept
: result_(std::move(r)),
          rowsNumber_(result_ ? mysql_num_rows(result_.get()) : 0),
          fieldArray_(result_ ? mysql_fetch_fields(result_.get()) : nullptr),
          fieldsNumber_(result_ ? mysql_num_fields(result_.get()) : 0),
          affectedRows_(affectedRows),
          insertId_(insertId)
{
    if (fieldsNumber_ > 0)
    {
        fieldsMapPtr_ = std::make_shared<
            std::unordered_map<std::string, RowSizeType>>();
        for (RowSizeType i = 0; i < fieldsNumber_; ++i)
        {
            std::string fieldName = fieldArray_[i].name;
            std::transform(fieldName.begin(),
                           fieldName.end(),
                           fieldName.begin(),
                           [](unsigned char c) { return tolower(c); });
            (*fieldsMapPtr_)[fieldName] = i;
        }
    }
    if (size() > 0)
    {
        rowsPtr_ = std::make_shared<
            std::vector<std::pair<char **, std::vector<unsigned long>>>>();
        MYSQL_ROW row;
        std::vector<unsigned long> vLens;
        vLens.resize(fieldsNumber_);
        while ((row = mysql_fetch_row(result_.get())) != NULL)
        {
            auto lengths = mysql_fetch_lengths(result_.get());
            memcpy(vLens.data(),
                   lengths,
                   sizeof(unsigned long) * fieldsNumber_);
            rowsPtr_->push_back(std::make_pair(row, vLens));
        }
    }
}

Result::SizeType MySQLResultImpl::size() const noexcept
{
    return rowsNumber_;
}

Result::RowSizeType MySQLResultImpl::columns() const noexcept
{
    return fieldsNumber_;
}

const char *MySQLResultImpl::columnName(RowSizeType number) const
{
    assert(number < fieldsNumber_);
    if (fieldArray_)
        return fieldArray_[number].name;
    return "";
}

Result::SizeType MySQLResultImpl::affectedRows() const noexcept
{
    return affectedRows_;
}

Result::RowSizeType MySQLResultImpl::columnNumber(const char colName[]) const
{
    if (!fieldsMapPtr_)
        return -1;
    std::string col(colName);
    std::transform(col.begin(), col.end(), col.begin(), [](unsigned char c) {
        return tolower(c);
    });
    if (fieldsMapPtr_->find(col) != fieldsMapPtr_->end())
        return (*fieldsMapPtr_)[col];
    throw RangeError(std::string("no column named ") + colName);
}

const char *MySQLResultImpl::getValue(SizeType row, RowSizeType column) const
{
    if (rowsNumber_ == 0 || fieldsNumber_ == 0)
        return NULL;
    assert(row < rowsNumber_);
    assert(column < fieldsNumber_);
    return (*rowsPtr_)[row].first[column];
}

bool MySQLResultImpl::isNull(SizeType row, RowSizeType column) const
{
    return getValue(row, column) == NULL;
}

Result::FieldSizeType MySQLResultImpl::getLength(SizeType row,
                                                 RowSizeType column) const
{
    if (rowsNumber_ == 0 || fieldsNumber_ == 0)
        return 0;
    assert(row < rowsNumber_);
    assert(column < fieldsNumber_);
    return (*rowsPtr_)[row].second[column];
}

unsigned long long MySQLResultImpl::insertId() const noexcept
{
    return insertId_;
}
