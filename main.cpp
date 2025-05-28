#include <iostream>
#include <mutex>
#include <fmt/format.h>

// TIP 要<b>Run</b>代码，请按 <shortcut actionId="Run"/> 或点击装订区域中的 <icon src="AllIcons.Actions.Execute"/> 图标。
int main() {

    std::string s = fmt::format("{0} {1}, {0}!", "'Hello'", "world");
    std::cout<<s<<std::endl;
}