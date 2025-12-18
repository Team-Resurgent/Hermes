#pragma once

#include "external.h"

class IsoChecker
{
public:
    static bool ContainsCheat();
	static std::vector<std::string> ValidateIsosInPath(std::string path, bool isLegacy, bool &isCci);
};
