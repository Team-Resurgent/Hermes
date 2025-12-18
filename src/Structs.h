#pragma once

#include "Defines.h"
#include "Integers.h"

struct CUSTOM_LAUNCH_ISO_DATA
{
	uint32_t magic;	
	char mountPath[MAX_PATHNAME];
};