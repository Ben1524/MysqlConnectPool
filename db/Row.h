//
// Created by cxk_zjq on 25-5-27.
//

#ifndef ROW_H
#define ROW_H
#pragma once

#include "Result.h"

namespace cxk
{
class Field;                // 字段类前向声明
class ConstRowIterator;     // 常量行迭代器前向声明
class ConstReverseRowIterator; // 常量反向行迭代器前向声明

/**
 * @brief 数据库结果行类
 * 表示数据库结果集中的一行数据，提供字段访问和迭代功能
 */
class Row
{
public:
    using SizeType = size_t;           ///< 大小类型（用于索引和长度）
    using Reference = Field;           ///< 字段引用类型
    using ConstIterator = ConstRowIterator; ///< 常量迭代器类型（只读）
    using Iterator = ConstIterator;    ///< 迭代器类型（与常量迭代器相同，均为只读）
    using ConstReverseIterator = ConstReverseRowIterator; ///< 常量反向迭代器类型

    using DifferenceType = long;       ///< 迭代器差值类型

    /**
     * @brief 通过索引访问字段（无越界检查）
     * @param index 字段索引（从0开始）
     * @return 对应索引的字段引用
     */
    Reference operator[](SizeType index) const noexcept;

    /**
     * @brief 通过索引访问字段（支持int类型索引，无越界检查）
     * @param index 字段索引（从0开始）
     * @return 对应索引的字段引用
     */
    Reference operator[](int index) const noexcept;

    /**
     * @brief 通过列名访问字段（C风格字符串）
     * @param columnName 列名
     * @return 对应列名的字段引用
     * @throw 异常：列名不存在时抛出
     */
    Reference operator[](const char columnName[]) const;

    /**
     * @brief 通过列名访问字段（C++字符串）
     * @param columnName 列名
     * @return 对应列名的字段引用
     * @throw 异常：列名不存在时抛出
     */
    Reference operator[](const std::string &columnName) const;

    /**
     * @brief 通过索引访问字段（带越界检查）
     * @param index 字段索引（从0开始）
     * @return 对应索引的字段引用
     * @throw 异常：索引越界时抛出
     */
    Reference at(SizeType index) const;

    /**
     * @brief 通过列名访问字段（C风格字符串，带越界检查）
     * @param columnName 列名
     * @return 对应列名的字段引用
     * @throw 异常：列名不存在时抛出
     */
    Reference at(const char columnName[]) const;

    /**
     * @brief 通过列名访问字段（C++字符串，带越界检查）
     * @param columnName 列名
     * @return 对应列名的字段引用
     * @throw 异常：列名不存在时抛出
     */
    Reference at(const std::string &columnName) const;

    /**
     * @brief 获取行中的字段数量
     * @return 字段总数
     */
    SizeType size() const;

    /**
     * @brief 获取行的容量（与字段数相同）
     * @return 容量（等于size()）
     */
    SizeType capacity() const noexcept
    {
        return size();
    }

    /**
     * @brief 获取正向迭代器（指向首字段）
     * @return 指向首字段的常量迭代器
     */
    ConstIterator begin() const noexcept;

    /**
     * @brief 获取正向常量迭代器（指向首字段）
     * @return 指向首字段的常量迭代器（与begin()相同）
     */
    ConstIterator cbegin() const noexcept;

    /**
     * @brief 获取正向迭代器（指向末字段的下一个位置）
     * @return 指向末字段之后的常量迭代器
     */
    ConstIterator end() const noexcept;

    /**
     * @brief 获取正向常量迭代器（指向末字段的下一个位置）
     * @return 指向末字段之后的常量迭代器（与end()相同）
     */
    ConstIterator cend() const noexcept;

    /**
     * @brief 获取反向迭代器（指向末字段）
     * @return 指向末字段的常量反向迭代器
     */
    ConstReverseIterator rbegin() const;

    /**
     * @brief 获取反向常量迭代器（指向末字段）
     * @return 指向末字段的常量反向迭代器（与rbegin()相同）
     */
    ConstReverseIterator crbegin() const;

    /**
     * @brief 获取反向迭代器（指向首字段的前一个位置）
     * @return 指向首字段之前的常量反向迭代器
     */
    ConstReverseIterator rend() const;

    /**
     * @brief 获取反向常量迭代器（指向首字段的前一个位置）
     * @return 指向首字段之前的常量反向迭代器（与rend()相同）
     */
    ConstReverseIterator crend() const;

#ifdef _MSC_VER
    Row() noexcept = default; // MSVC兼容默认构造函数
#endif

    // 拷贝构造、移动构造和赋值运算符默认实现
    Row(const Row &r) noexcept = default;
    Row(Row &&) noexcept = default;
    Row &operator=(const Row &) noexcept = default;

private:
    Result result_; ///< 所属的结果集对象，注意Result类保存了一个ResultImpl指针，复制是为浅拷贝，共享数据

protected:
    friend class Field; ///< Field类可访问内部接口

    /**
     * 行号（内部使用）
     * 预期为size_t类型，但由于反向迭代器与正向迭代器的关系，允许下溢到-1
     */
    long index_{0};
    SizeType end_{0}; ///< 行数据结束位置（内部索引）

    friend class Result; ///< Result类可访问构造函数

    /**
     * @brief 构造函数（内部使用）
     * @param r 所属的结果集对象
     * @param index 行索引
     */
    Row(const Result &r, SizeType index) noexcept;
};

} // namespace cxk

#endif //ROW_H