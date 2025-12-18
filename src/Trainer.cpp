#include "Trainer.h"
#include "Defines.h"
#include "Structs.h"
#include "Path.h"
#include "External.h"

void Trainer::LaunchTrainerMenuIfExists()
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK iostatusBlock;

    char menuPath[MAX_PATHNAME];
    sprintf(menuPath, "%s\\default.xbe", HERMES_PATH);
	STRING sMenuPath = { (USHORT)strlen(menuPath), (USHORT)strlen(menuPath) + 1, menuPath};
    objectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
    objectAttributes.ObjectName = &sMenuPath;
    objectAttributes.RootDirectory = 0;

    status = NtOpenFile(&fileHandle, GENERIC_READ | SYNCHRONIZE, &objectAttributes, &iostatusBlock, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);
    if (status != 0) 
    {
        return;
    }
    NtClose(fileHandle);

    char mountPoint[MAX_PATHNAME];
    sprintf(mountPoint, "\\??\\D:");
	STRING sMountPoint = { (USHORT)strlen(mountPoint), (USHORT)strlen(mountPoint) + 1, mountPoint};
    IoDeleteSymbolicLink(&sMountPoint);

    char systemPath[MAX_PATHNAME];
    sprintf(systemPath, HERMES_PATH);
    STRING sSystemPath = {(USHORT)strlen(systemPath), (USHORT)strlen(systemPath) + 1, systemPath};
    status = IoCreateSymbolicLink(&sMountPoint, &sSystemPath);
    if (status != 0) 
    {
        return;
    }

    CUSTOM_LAUNCH_ISO_DATA launchData;
    launchData.magic = CUSTOM_LAUNCH_ISO_MAGIC;
    sprintf(launchData.mountPath, Path::GetAppPath().c_str());
    XWriteTitleInfoAndRebootA("default.xbe", HERMES_PATH, LDT_TITLE, 0x5452ffff, (PLAUNCH_DATA)&launchData);
}