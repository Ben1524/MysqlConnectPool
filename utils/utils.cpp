//
// Created by cxk_zjq on 25-5-27.
//

#include "utils.h"
#include <cassert>

namespace utils
{
    std::string hexToBinaryString(const char *ptr, size_t length)
    {
        assert(length % 2 == 0);
        std::string ret(length / 2, '\0');
        for (size_t i = 0; i < ret.length(); ++i)
        {
            auto p = i * 2;
            char c1 = ptr[p];
            if (c1 >= '0' && c1 <= '9')
            {
                c1 -= '0';
            }
            else if (c1 >= 'a' && c1 <= 'f')
            {
                c1 -= 'a';
                c1 += 10;
            }
            else if (c1 >= 'A' && c1 <= 'F')
            {
                c1 -= 'A';
                c1 += 10;
            }
            else
            {
                return "";
            }
            char c2 = ptr[p + 1];
            if (c2 >= '0' && c2 <= '9')
            {
                c2 -= '0';
            }
            else if (c2 >= 'a' && c2 <= 'f')
            {
                c2 -= 'a';
                c2 += 10;
            }
            else if (c2 >= 'A' && c2 <= 'F')
            {
                c2 -= 'A';
                c2 += 10;
            }
            else
            {
                return "";
            }
            ret[i] = c1 * 16 + c2;
        }
        return ret;
    }


    std::vector<char> hexToBinaryVector(const char *ptr, size_t length)
    {
        assert(length % 2 == 0);
        std::vector<char> ret(length / 2, '\0');
        for (size_t i = 0; i < ret.size(); ++i)
        {
            auto p = i * 2;
            char c1 = ptr[p];
            if (c1 >= '0' && c1 <= '9')
            {
                c1 -= '0';
            }
            else if (c1 >= 'a' && c1 <= 'f')
            {
                c1 -= 'a';
                c1 += 10;
            }
            else if (c1 >= 'A' && c1 <= 'F')
            {
                c1 -= 'A';
                c1 += 10;
            }
            else
            {
                return std::vector<char>();
            }
            char c2 = ptr[p + 1];
            if (c2 >= '0' && c2 <= '9')
            {
                c2 -= '0';
            }
            else if (c2 >= 'a' && c2 <= 'f')
            {
                c2 -= 'a';
                c2 += 10;
            }
            else if (c2 >= 'A' && c2 <= 'F')
            {
                c2 -= 'A';
                c2 += 10;
            }
            else
            {
                return std::vector<char>();
            }
            ret[i] = c1 * 16 + c2;
        }
        return ret;
    }

    std::vector<std::string> splitString(const std::string& s, const std::string& delimiter, bool acceptEmptyString)
    {
        if (delimiter.empty())
            return std::vector<std::string>{};
        std::vector<std::string> v;
        size_t last = 0;
        size_t next = 0;
        while ((next = s.find(delimiter, last)) != std::string::npos)
        {
            if (next > last || acceptEmptyString)
                v.push_back(s.substr(last, next - last));
            last = next + delimiter.length();
        }
        if (s.length() > last || acceptEmptyString)
            v.push_back(s.substr(last));
        return v;
    }
}
