/**
*@ClassName ScopeExit
*@Author cxk
*@Data 25-6-7 下午5:02
*/
//

#ifndef SCOPEEXIT_H
#define SCOPEEXIT_H


/**
 * @brief 一个常用的RAII模式的作用域退出类，用于在作用域结束时自动执行一个函数。
 */
namespace utils
{

template <typename F>
struct ScopeExit
{
    ScopeExit(F &&f) : f_(std::forward<F>(f))
    {
    }
    ~ScopeExit()
    {
        f_();
    }
    F f_;
};

template <typename F>
ScopeExit<F> makeScopeExit(F &&f)
{
    return ScopeExit<F>(std::forward<F>(f));
};
}  // namespace

#endif //SCOPEEXIT_H
