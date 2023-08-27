#include "debug.h"

//static function
void debug::print(const CHAR* buf, ...)
{
    CHAR strBuffer[1024];

    va_list arglist;
    va_start( arglist, buf );
    _vsnprintf( strBuffer, sizeof(strBuffer), buf, arglist );
    va_end( arglist );

    strBuffer[sizeof(strBuffer)-1] = '\0';
    OutputDebugStringA( strBuffer );
}
