#include "Path.h"
#include "Defines.h"
#include "External.h"

std::string Path::GetAppPath()
{
	char path[MAX_PATHNAME];
    int pathlen;

    memset(path, 0, MAX_PATHNAME);
    strncpy(path, XeImageFileName->Buffer, XeImageFileName->Length < (MAX_PATHNAME - 1) ? XeImageFileName->Length : (MAX_PATHNAME - 1));
    pathlen = strrchr(path, '\\') - path;
    path[pathlen] = '\0';

	return std::string(path);
}