#include "debug.h"
#include "Defines.h"

#include <xtl.h>
#include <string>

void Debug::Print(const char* message, ...)
{
    char buffer[1024];

    va_list arglist;
    va_start(arglist, message);
    _vsnprintf(buffer, DEBUG_PRINT_BUFFER, message, arglist);
    va_end(arglist);

    buffer[DEBUG_PRINT_BUFFER - 1] = '\0';
    OutputDebugStringA(buffer);
}
