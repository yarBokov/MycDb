#ifndef __COMMON_STRINGS_METHODS_H
#define __COMMON_STRINGS_METHODS_H

#include <string>
#include <algorithm>

namespace common
{
    void trim_str(std::string &s) 
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    }
}

#endif