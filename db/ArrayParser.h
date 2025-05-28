//
// Created by cxk_zjq on 25-5-27.
//

#ifndef ARRAYPARSER_H
#define ARRAYPARSER_H

#include <stdexcept>
#include <string>
#include <utility>

namespace cxk
{
    /// 底层数组解析器
    /** 用于读取从数据库检索到的数组字段。
     *
     * 仅在客户端编码为UTF-8、ASCII或作为ASCII超集的单字节编码时使用。
     *
     * 输入是包含数据库返回的数组文本表示的C风格字符串。解析器动态读取该表示。
     * 字符串必须在内存中保持直到解析完成。
     *
     * 通过调用@c getNext解析数组，直到返回@c juncture为"done"。
     * @c juncture指示解析器在该步骤中找到的内容：数组是“嵌套”到更深层次，还是“取消嵌套”回退。
     */
    class ArrayParser
    {
    public:
        /// 数组中最新找到的内容类型
        enum juncture
        {
            /// 开始新行
            row_start,
            /// 结束当前行
            row_end,
            /// 找到空值（NULL）
            null_value,
            /// 找到字符串值
            string_value,
            /// 解析完成
            done,
        };

        /// 构造函数。无需直接使用此构造函数，应使用@c field::as_array代替。
        explicit ArrayParser(const char input[]);

        /// 解析数组中的下一步内容
        /** 返回解析到的内容。如果@c juncture为@c string_value，字符串将包含对应值；
         * 否则，字符串为空。
         *
         * 持续调用直至返回的@c juncture为@c done。
         */
        std::pair<juncture, std::string> getNext();

    private:
        /// 输入字符串中的当前解析位置
        const char *pos_;
    };

}  // namespace orm



#endif //ARRAYPARSER_H