//
// Created by cxk_zjq on 25-5-27.
//

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace utils
{
    std::string hexToBinaryString(const char *ptr, size_t length);
    std::vector<char> hexToBinaryVector(const char *ptr, size_t length);
    std::vector<std::string> splitString(const std::string &s,
                                            const std::string &delimiter,
                                            bool acceptEmptyString = false);
}


#endif //UTILS_H
