//
// Created by cxk_zjq on 25-6-2.
//

#include "MysqlResultImpl.h"
#include <codecvt>
#include <string>
#include <locale>
#include <codecvt>

namespace cxk
{




const char* convertChar16ToChar(const char16_t* char16Str) {
    if (!char16Str) return nullptr;
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    static std::string result;
    try {
        result = converter.to_bytes(char16Str);
    } catch (const std::range_error&) {
        // 转换失败时返回空字符串
        result.clear();
    }
    return result.c_str();
}


Result::SizeType MysqlResultImpl::size() const noexcept
{
    return rowsNumber_;
}

Result::RowSizeType MysqlResultImpl::columns() const noexcept
{
    return fieldsNumber_;
}

const char *MysqlResultImpl::columnName(RowSizeType number) const
{
    assert(number < fieldsNumber_);
    if (fieldArray_.empty()== false)
    {
         return convertChar16ToChar(fieldArray_[number].getColumnName().c_str());
        // 这里假设fieldArray_是一个存储字段信息的容器，且每个字段有getColumnName()方法返回char16_t*
    }
//        return fieldArray_[number].getColumnName().c_str();
    return "";
}

Result::SizeType MysqlResultImpl::affectedRows() const noexcept
{
    return affectedRows_;
}

/// @brief 获取指定列名的列索引
Result::RowSizeType MysqlResultImpl::columnNumber(const char colName[]) const
{
    if (!fieldsMapPtr_)
        return -1;
    std::string col(colName);
    std::transform(col.begin(), col.end(), col.begin(), [](unsigned char c) {
        return tolower(c);
    });
    if (fieldsMapPtr_->find(col) != fieldsMapPtr_->end())
        return (*fieldsMapPtr_)[col];
    throw std::runtime_error("Column not found: " + col);
}

const char *MysqlResultImpl::getValue(SizeType row, RowSizeType column) const
{
    if (rowsNumber_ == 0 || fieldsNumber_ == 0)
        return NULL;
    assert(row < rowsNumber_);
    assert(column < fieldsNumber_);
    return (*rowsPtr_)[row].first[column];  // 返回指定行和列的值
}

bool MysqlResultImpl::isNull(SizeType row, RowSizeType column) const
{
    return getValue(row, column) == NULL;
}

Result::FieldSizeType MysqlResultImpl::getLength(SizeType row,
                                                 RowSizeType column) const
{
    if (rowsNumber_ == 0 || fieldsNumber_ == 0)
        return 0;
    assert(row < rowsNumber_);
    assert(column < fieldsNumber_);
    return (*rowsPtr_)[row].second[column];
}

unsigned long long MysqlResultImpl::insertId() const noexcept
{
    return insertId_;
}


} // cxk