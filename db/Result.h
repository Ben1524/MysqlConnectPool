//
// Created by cxk_zjq on 25-5-27.
//

#ifndef RESULT_H
#define RESULT_H

#pragma once
#include <memory>
#include <string>
#include <future>
#include <algorithm>
#include <assert.h>

// 取自libpqxx并修改
// libpqxx的许可证见COPYING文件
namespace cxk
{
    class ConstResultIterator;          // 常量结果迭代器
    class ConstReverseResultIterator;   // 常量反向结果迭代器
    class Row;                          // 行数据类
    class ResultImpl;                   // 结果实现类（内部使用）
    using ResultImplPtr = std::shared_ptr<ResultImpl>;  // 结果实现智能指针


enum class SqlStatus
{
    Ok,
    End
};
/**
 * @brief 数据库查询结果集类
 * 表示数据库查询返回的结果集，提供迭代器、行列访问等功能
 */
class Result
{
  public:
    explicit Result(ResultImplPtr ptr) : resultPtr_(std::move(ptr))
    {
    }

    Result(const Result &r) noexcept = default;
    Result(Result &&) noexcept = default;
    Result &operator=(const Result &r) noexcept;
    Result &operator=(Result &&) noexcept;
    using DifferenceType = long;
    using SizeType = size_t;
    using Reference = Row;
    using ConstIterator = ConstResultIterator;
    using Iterator = ConstIterator;
    using RowSizeType = unsigned long;
    using FieldSizeType = unsigned long;

    using ConstReverseIterator = ConstReverseResultIterator;
    using ReverseIterator = ConstReverseIterator;

    using value_type = Row;
    using size_type = SizeType;
    using difference_type = DifferenceType;
    using reference = Reference;
    using const_reference = const Reference;
    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using reverse_iterator = ConstReverseIterator;
    using const_reverse_iterator = ConstReverseIterator;

    /**
     * @brief 获取结果集的行数
     * @return 结果集包含的行数
     */
    SizeType size() const noexcept;

    /**
     * @brief 获取结果集的容量（与行数相同）
     * @return 结果集容量（等于size()）
     */
    SizeType capacity() const noexcept
    {
        return size();
    }

    /**
     * @brief 获取正向迭代器（指向首行）
     * @return 指向首行的常量迭代器
     */
    ConstIterator begin() const noexcept;

    /**
     * @brief 获取正向常量迭代器（指向首行）
     * @return 指向首行的常量迭代器（与begin()相同）
     */
    ConstIterator cbegin() const noexcept;

    /**
     * @brief 获取正向迭代器（指向末行的下一个位置）
     * @return 指向末行之后的常量迭代器
     */
    ConstIterator end() const noexcept;

    /**
     * @brief 获取正向常量迭代器（指向末行的下一个位置）
     * @return 指向末行之后的常量迭代器（与end()相同）
     */
    ConstIterator cend() const noexcept;

    /**
     * @brief 获取反向迭代器（指向末行）
     * @return 指向末行的常量反向迭代器
     */
    ConstReverseIterator rbegin() const;

    /**
     * @brief 获取反向常量迭代器（指向末行）
     * @return 指向末行的常量反向迭代器（与rbegin()相同）
     */
    ConstReverseIterator crbegin() const;

    /**
     * @brief 获取反向迭代器（指向首行的前一个位置）
     * @return 指向首行之前的常量反向迭代器
     */
    ConstReverseIterator rend() const;

    /**
     * @brief 获取反向常量迭代器（指向首行的前一个位置）
     * @return 指向首行之前的常量反向迭代器（与rend()相同）
     */
    ConstReverseIterator crend() const;

    /**
     * @brief 判断结果集是否为空
     * @return true表示结果集为空，false表示非空
     */
    bool empty() const noexcept
    {
        return size() == 0;
    }

    /**
     * @brief 获取首行数据
     * @return 结果集的首行数据（不可为空）
     */
    Reference front() const noexcept;

    /**
     * @brief 获取末行数据
     * @return 结果集的末行数据（不可为空）
     */
    Reference back() const noexcept;

    /**
     * @brief 通过索引访问行数据（不检查越界）
     * @param index 行索引（从0开始）
     * @return 对应索引的行数据
     */
    Reference operator[](SizeType index) const noexcept;

    /**
     * @brief 通过索引访问行数据（检查越界）
     * @param index 行索引（从0开始）
     * @return 对应索引的行数据
     * @throw 异常：索引越界时抛出
     */
    Reference at(SizeType index) const;

    /**
     * @brief 交换两个结果集的数据
     * @param rhs 待交换的结果集对象
     */
    void swap(Result &rhs) noexcept;

    /**
     * @brief 获取结果集的列数
     * @return 每行包含的字段数量
     */
    RowSizeType columns() const noexcept;

    /**
     * @brief 获取指定列的列名（通过列号）
     * @param number 列号（从0开始）
     * @return 对应列的名称
     * @throw 异常：列号无效时抛出
     */
    const char *columnName(RowSizeType number) const;

    /**
     * @brief 获取受影响的行数（针对INSERT/UPDATE/DELETE操作）
     * @return 操作影响的行数，其他操作返回0
     */
    SizeType affectedRows() const noexcept;

    /**
     * @brief 获取插入操作的自增主键ID（仅适用于MySQL/SQLite3）
     * @return 自增主键ID，PostgreSQL始终返回0（需通过RETURNING子句获取）
     */
    unsigned long long insertId() const noexcept;

#ifdef _MSC_VER
    Result() noexcept = default; // MSVC兼容默认构造函数
#endif

private:
    ResultImplPtr resultPtr_; // 结果实现类的智能指针（内部数据存储）

    friend class Field;       // Field类可访问内部接口
    friend class Row;         // Row类可访问内部接口

    /**
     * @brief 通过列名获取列号（内部使用）
     * @param colName 列名（C风格字符串）
     * @return 对应的列号
     * @throw 异常：列名不存在时抛出
     */
    RowSizeType columnNumber(const char colName[]) const;

    /**
     * @brief 通过列名获取列号（内部使用，C++字符串版本）
     * @param name 列名
     * @return 对应的列号（调用columnNumber(colName.c_str())）
     */
    RowSizeType columnNumber(const std::string &name) const
    {
        return columnNumber(name.c_str());
    }

    /**
     * @brief 获取列的OID（仅适用于PostgreSQL）
     * @param column 列号
     * @return 列的OID，其他数据库返回0
     */
    int oid(RowSizeType column) const noexcept;

    /**
     * @brief 获取指定行/列的字段值（内部使用）
     * @param row 行号
     * @param column 列号
     * @return 字段值的C风格字符串（NULL表示空值）
     */
    const char *getValue(SizeType row, RowSizeType column) const;

    /**
     * @brief 判断指定行/列的字段是否为NULL（内部使用）
     * @param row 行号
     * @param column 列号
     * @return true表示字段值为NULL，false表示有有效值
     */
    bool isNull(SizeType row, RowSizeType column) const;

    /**
     * @brief 获取指定行/列字段值的长度（内部使用）
     * @param row 行号
     * @param column 列号
     * @return 字段值的字节长度（不包含NULL终止符）
     */
    FieldSizeType getLength(SizeType row, RowSizeType column) const;
};

inline void swap(Result &one, Result &two) noexcept
{
    one.swap(two);
}
}  // namespace cxk
#ifndef _MSC_VER
namespace std
{
template <>
inline void swap(cxk::Result &one, cxk::Result &two) noexcept
{
    one.swap(two);
}
}  // namespace std
#endif




#endif //RESULT_H
