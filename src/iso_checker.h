#pragma once

#include "external.h"

class iso_checker
{
public:
	static std::string getPath();
	static std::vector<std::string> validateIsosInPath(std::string path, bool isLegacy, bool &isCci);
private:

};
