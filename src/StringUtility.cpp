#include "StringUtility.h"

#include <algorithm>

std::string StringUtility::LowerCase(std::string const& value)
{
	std::string result = std::string(value);
	std::transform(result.begin(), result.end(), result.begin(), tolower);
	return result;
}

bool StringUtility::EndsWith(std::string const& value, std::string const& ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::vector<std::string> StringUtility::Split(std::string const& value, char const& delimiter)
{
	std::string buffer;
	std::vector<std::string> items;
	for (size_t i = 0; i < value.size(); i++)
	{
		if (value.at(i) == delimiter)
		{
			items.push_back(buffer);
			buffer.clear();
			continue;
		}
		buffer.push_back(value.at(i));
	}
	if (!buffer.empty())
	{
		items.push_back(buffer);
	}
	return items;
}