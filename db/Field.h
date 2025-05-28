//
// Created by cxk_zjq on 25-5-27.
//

#ifndef FIELD_H
#define FIELD_H
#pragma once

#include <string_view>
#include "ArrayParser.h"
#include "Result.h"    // 假设Result和Row在当前命名空间或已正确引入
#include "Row.h"
#include <spdlog/spdlog.h>    // 引入spdlog头文件
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#ifdef __linux__
#include <arpa/inet.h>
#endif

namespace cxk
{
/**
 * @brief 数据库查询结果字段类
 * 表示数据库结果集中的单个字段，提供类型转换和数组解析功能
 */
class Field
{
public:
    using SizeType = unsigned long;

    /**
     * @brief 获取列名
     * @return 列名的C风格字符串
     */
    const char *name() const;

    /**
     * @brief 判断字段值是否为NULL
     * @return true表示值为NULL，false表示有有效值
     */
    bool isNull() const;

    /**
     * @brief 获取字段值的C风格字符串表示
     * 字段数据以NULL结尾的C字符串形式存储，这是最快的读取方式
     * 可通过to()或as()函数转换为其他类型（如int、C++字符串等）
     * @return 字段值的C风格字符串
     */
    const char *c_str() const;

    /**
     * @brief 获取C风格字符串的长度
     * @return 字符串长度
     */
    size_t length() const
    {
        return result_.getLength(row_, column_);
    }

    /**
     * @brief 将字段值转换为指定类型T的值
     * @tparam T 目标类型
     * @return 转换后的类型值，若值为NULL则返回默认构造值
     */
    template <typename T>
    T as() const
    {
        if (isNull())
            return T();
        auto data_ = result_.getValue(row_, column_);
        T value = T();
        if (data_)
        {
            try
            {
                std::stringstream ss(data_);
                ss >> value;
            }
            catch (...)
            {
                spdlog::debug("类型转换错误");    // 使用spdlog输出调试日志
            }
        }
        return value;
    }

    /**
     * @brief 解析字段为SQL数组
     * 获取数组解析器以从数组中提取值和结构
     * 注意：在解析完成前需确保result对象存活
     * @return 数组解析器对象
     */
    ArrayParser getArrayParser() const
    {
        return ArrayParser(result_.getValue(row_, column_));
    }

    /**
     * @brief 将字段值解析为指定类型的数组（智能指针版本）
     * @tparam T 数组元素类型
     * @return 包含解析结果的shared_ptr数组
     *          若为NULL值则对应元素为nullptr
     */
    template <typename T>
    std::vector<std::shared_ptr<T>> asArray() const
    {
        std::vector<std::shared_ptr<T>> ret;
        auto arrParser = getArrayParser();
        while (1)
        {
            auto arrVal = arrParser.getNext();
            if (arrVal.first == ArrayParser::juncture::done)
            {
                break;
            }
            if (arrVal.first == ArrayParser::juncture::string_value)
            {
                T val;
                std::stringstream ss(std::move(arrVal.second));
                ss >> val;
                ret.push_back(std::make_shared<T>(val));    // 使用make_shared优化
            }
            else if (arrVal.first == ArrayParser::juncture::null_value)
            {
                ret.push_back(nullptr);
            }
        }
        return ret;
    }

protected:
    Result::SizeType row_;          ///< 行号
    long column_;                   ///< 列号（允许下溢到-1以支持反向迭代器）
    friend class Row;                ///< Row类可访问构造函数

    /**
     * @brief 构造函数（友元类专用）
     * @param row 所属的行对象
     * @param columnNum 列号
     */
    Field(const Row &row, Row::SizeType columnNum) noexcept;

private:
    const Result result_;           ///< 所属的结果集对象
};

// 类型特化声明（需确保DROGON_EXPORT正确定义或替换为当前项目的导出宏）
template <>
std::string Field::as<std::string>() const;
template <>
const char *Field::as<const char *>() const;
template <>
char *Field::as<char *>() const;
template <>
std::vector<char> Field::as<std::vector<char>>() const;

// 具体类型的转换实现
template <>
inline std::string_view Field::as<std::string_view>() const
{
    auto first = result_.getValue(row_, column_);
    auto length = result_.getLength(row_, column_);
    return {first, length};
}

template <>
inline float Field::as<float>() const
{
    if (isNull())
        return 0.0f;
    return std::stof(result_.getValue(row_, column_));
}

template <>
inline double Field::as<double>() const
{
    if (isNull())
        return 0.0;
    return std::stod(result_.getValue(row_, column_));
}

template <>
inline bool Field::as<bool>() const
{
    if (result_.getLength(row_, column_) != 1)
    {
        return false;
    }
    auto value = result_.getValue(row_, column_);
    return (*value == 't' || *value == '1');    // 简化条件判断
}

template <>
inline int Field::as<int>() const
{
    if (isNull())
        return 0;
    return std::stoi(result_.getValue(row_, column_));
}

template <>
inline long Field::as<long>() const
{
    if (isNull())
        return 0L;
    return std::stol(result_.getValue(row_, column_));
}

template <>
inline int8_t Field::as<int8_t>() const
{
    if (isNull())
        return 0;
    return static_cast<int8_t>(atoi(result_.getValue(row_, column_)));
}

template <>
inline long long Field::as<long long>() const
{
    if (isNull())
        return 0LL;
    return atoll(result_.getValue(row_, column_));
}

template <>
inline unsigned int Field::as<unsigned int>() const
{
    if (isNull())
        return 0u;
    return static_cast<unsigned int>(std::stoul(result_.getValue(row_, column_)));
}

template <>
inline unsigned long Field::as<unsigned long>() const
{
    if (isNull())
        return 0ul;
    return std::stoul(result_.getValue(row_, column_));
}

template <>
inline uint8_t Field::as<uint8_t>() const
{
    if (isNull())
        return 0;
    return static_cast<uint8_t>(atoi(result_.getValue(row_, column_)));
}

template <>
inline unsigned long long Field::as<unsigned long long>() const
{
    if (isNull())
        return 0ull;
    return std::stoull(result_.getValue(row_, column_));
}

}  // namespace cxk

#endif //FIELD_H