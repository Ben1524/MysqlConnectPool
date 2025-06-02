
#include <codecvt>
#include <string>
#include <locale>
#include <codecvt>
#include <string>

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

int main()
{
    const char16_t* char16Str = u"Hello, World!";
    const char* result = convertChar16ToChar(char16Str);
    if (result) {
        printf("Converted string: %s\n", result);
    } else {
        printf("Conversion failed.\n");
    }
    return 0;
}