#pragma once

#include <string>
#include <vector>

class StringUtility
{
public:
    static std::string StringUtility::LowerCase(std::string const& value);
    static bool StringUtility::EndsWith(std::string const& value, std::string const& ending);
    static std::vector<std::string> StringUtility::Split(std::string const& value, char const& delimiter);
};
