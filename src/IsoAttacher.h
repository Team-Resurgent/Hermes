#pragma once

#include "Integers.h"

#include <string>
#include <vector>

class IsoAttacher
{
public:
    static void AttachLegacy(std::string path, std::vector<std::string> isos, uint16_t build);
    static void AttachCerbios(std::string path, std::vector<std::string> isos, bool isCci);
};
