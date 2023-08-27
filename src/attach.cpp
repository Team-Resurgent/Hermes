#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include "iso_checker.h"
#include "debug.h"
#include "external.h"

void cerbiosAttach(std::string path, std::vector<std::string> isos, bool isCci)
{
	// Allocate some mem to use

	void *membuf = NULL;
    unsigned long membuf_size = 1024*1024;

    NTSTATUS status = NtAllocateVirtualMemory(&membuf, 0, &membuf_size, MEM_COMMIT | MEM_NOZERO, PAGE_READWRITE);
    if (!NT_SUCCESS(status)) 
	{
		debug::print("Attach: Failed To Allocate Memory.\r\n");
		HalReturnToFirmware(2);
    }

	// Populate attach structure

	ATTACH_SLICE_DATA_CERBIOS *asd = (ATTACH_SLICE_DATA_CERBIOS *)membuf;    
    memset(membuf, 0, sizeof(ATTACH_SLICE_DATA_CERBIOS));
	asd->DeviceType = isCci ? 'd' : 'D';
	
    char *strbuf = (char *)membuf + sizeof(ATTACH_SLICE_DATA_CERBIOS);
	membuf_size -= sizeof(ATTACH_SLICE_DATA_CERBIOS);

	_snprintf(strbuf, membuf_size, "\\Device\\CdRom0");

    RtlInitAnsiString(&asd->MountPoint, strbuf);
	asd->MountPoint.MaximumLength = asd->MountPoint.Length+1;

	strbuf += asd->MountPoint.MaximumLength;
    membuf_size -= asd->MountPoint.MaximumLength;

    for(int i = 0; i < (int)isos.size(); i++) 
	{
        _snprintf(strbuf, membuf_size, "%s\\%s", path.c_str(), isos[i].c_str());

        RtlInitAnsiString(&asd->SliceFile[i], strbuf);
		asd->SliceFile[i].MaximumLength = asd->SliceFile[i].Length+1;

        strbuf += asd->SliceFile[i].MaximumLength;
        membuf_size -= asd->SliceFile[i].MaximumLength;
        
        asd->SliceCount = i + 1;
    }

	// Connect virtual control device
	
	debug::print("Attach: Connect Virtual Control Device.\r\n");

	ANSI_STRING dev_name;
    RtlInitAnsiString(&dev_name, "\\Device\\Virtual0\\Image0");

	OBJECT_ATTRIBUTES obj_attr;
    obj_attr.RootDirectory = NULL;
    obj_attr.ObjectName = &dev_name;
    obj_attr.Attributes = OBJ_CASE_INSENSITIVE;

	HANDLE h;
	IO_STATUS_BLOCK io_status;
    status = NtOpenFile(&h, GENERIC_READ | SYNCHRONIZE, &obj_attr, &io_status, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);
    if (!NT_SUCCESS(status)) 
	{
        debug::print("Attach: Failed To Open Virtual Control Device.\r\n");
        HalReturnToFirmware(2); 
    }

	// Dismount all virtual images

    debug::print("Attach: Dismounting All Virtual Images.\r\n");
    status = NtDeviceIoControlFile(h, NULL, NULL, NULL, &io_status, IOCTL_VIRTUAL_DETACH, NULL, 0, NULL, 0);
    if (!NT_SUCCESS(status)) 
	{
        debug::print("Attach: Failed Detach IOCTL.\r\n");
        HalReturnToFirmware(2);
    }

	// Attach new virtual images

    debug::print("Attach: Attempting To Attach Game ISO Of %d Slices To \\Device\\CdRom0\r\n", isos.size()); 
	status = NtDeviceIoControlFile(h, NULL, NULL, NULL, &io_status, IOCTL_VIRTUAL_ATTACH, asd, sizeof(ATTACH_SLICE_DATA_CERBIOS), NULL, 0);
    if (!NT_SUCCESS(status)) 
	{
        debug::print("Attach: Failed Attach IOCTL.\r\n");
        HalReturnToFirmware(2);
    }

	// Finished

	NtClose(h);
	debug::print("Attach: Finished.\r\n");
    HalReturnToFirmware(2);
}

void legacyAttach(std::string path, std::vector<std::string> isos, USHORT build)
{
	// Allocate some mem to use

	void *membuf = NULL;
    unsigned long membuf_size = 1024*1024;

    NTSTATUS status = NtAllocateVirtualMemory(&membuf, 0, &membuf_size, MEM_COMMIT | MEM_NOZERO, PAGE_READWRITE);
    if (!NT_SUCCESS(status)) 
	{
		debug::print("Attach: Failed To Allocate Memory.\r\n");
		HalReturnToFirmware(2);
    }

	// Populate attach structure

	ATTACH_SLICE_DATA_LEGACY *asd = (ATTACH_SLICE_DATA_LEGACY *)membuf;
    memset(membuf, 0, sizeof(ATTACH_SLICE_DATA_LEGACY));

    char *strbuf = (char *)membuf + sizeof(ATTACH_SLICE_DATA_LEGACY);
    membuf_size -= sizeof(ATTACH_SLICE_DATA_LEGACY);

    for(int i = 0; i < (int)isos.size(); i++) 
	{
        _snprintf(strbuf, membuf_size, "%s\\%s", path.c_str(), isos[i].c_str());
        RtlInitAnsiString(&asd->SliceFile[i], strbuf);	

		asd->SliceFile[i].MaximumLength = asd->SliceFile[i].Length + 1;

        strbuf += asd->SliceFile[i].MaximumLength;
        membuf_size -= asd->SliceFile[i].MaximumLength;

        asd->SliceCount = i + 1;
    }

	// Connect virtual control device

	debug::print("Attach: Connect Virtual Control Device.\r\n");

	ANSI_STRING dev_name;
	RtlInitAnsiString(&dev_name,"\\Device\\CdRom1");

	OBJECT_ATTRIBUTES obj_attr;
    obj_attr.RootDirectory = NULL;
    obj_attr.ObjectName = &dev_name;
    obj_attr.Attributes = OBJ_CASE_INSENSITIVE;

	HANDLE h;
	IO_STATUS_BLOCK io_status;
    status = NtOpenFile(&h, GENERIC_READ | SYNCHRONIZE, &obj_attr, &io_status, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);
    if (!NT_SUCCESS(status)) 
	{
        debug::print("Attach: Failed To Open Virtual Control Device.\r\n");
        HalReturnToFirmware(2); 
    }

	// Dismount all virtual images

    debug::print("Attach: Dismounting All Virtual Images.\r\n");
    status = NtDeviceIoControlFile(h, NULL, NULL, NULL, &io_status, IOCTL_VIRTUAL_CDROM_DETACH, NULL, 0, NULL, 0);
    if (!NT_SUCCESS(status)) 
	{
        debug::print("Attach: Failed Detach IOCTL.\r\n");
        HalReturnToFirmware(2);
    }

    NtClose(h);

    status = IoDismountVolumeByName(&dev_name);
	status = NtOpenFile(&h, GENERIC_READ | SYNCHRONIZE, &obj_attr, &io_status, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
    if (!NT_SUCCESS(status)) 
	{
		HalReturnToFirmware(2);
    }

	// Attach new virtual images

    debug::print("Attach: Attempting To Attach Game ISO Of %d Slices To \\Device\\CdRom1\r\n", isos.size());
	status = NtDeviceIoControlFile(h, NULL, NULL, NULL, &io_status, IOCTL_VIRTUAL_CDROM_ATTACH, asd, sizeof(ATTACH_SLICE_DATA_LEGACY), NULL, 0);
    if (!NT_SUCCESS(status)) 
	{
        debug::print("Attach: Failed Attach IOCTL.\r\n");
        HalReturnToFirmware(2);
    }

	status = IoDismountVolumeByName(&dev_name);

	NtClose(h);
	debug::print("Attach: Finished.\r\n");
	
	// Check If Using IND Bios
	if (build == 5003 || build == 5004) 
	{
		// Remove Current Symbolic Link
		char MountPoint[16];
		sprintf(MountPoint, "\\??\\D:");
		STRING saMountPoint = {(USHORT)strlen(MountPoint), (USHORT)strlen(MountPoint) + 1, MountPoint};
		IoDeleteSymbolicLink(&saMountPoint);

		// Recreate Symbolic Link & Mount D: as CdRom0
		char mountPoint[260];
		sprintf(mountPoint, "\\??\\D:");
		char systemPath[260];
		sprintf(systemPath,"\\Device\\CdRom0");
		STRING ReMountPoint = { (USHORT)strlen(mountPoint), (USHORT)strlen(mountPoint) + 1, mountPoint };
		STRING ReSystemPath = { (USHORT)strlen(systemPath), (USHORT)strlen(systemPath) + 1, systemPath };
		IoCreateSymbolicLink(&ReMountPoint, &ReSystemPath);

		// Launch Default.Xbe From New Mount  
		XLaunchNewImage("D:\\default.xbe", NULL);
    } 
	else 
	{
        HalReturnToFirmware(2);
    }
}

void __cdecl main()
{
	std::string path = iso_checker::getPath();
	debug::print("Kernel Version [%u.%u.%u.%u].\r\n", XboxKrnlVersion->Major, XboxKrnlVersion->Minor, XboxKrnlVersion->Build, XboxKrnlVersion->Qfe);
	debug::print("Application Path %s \r\n", path.c_str());

	bool isCci = false;
	std::vector<std::string> isos = iso_checker::validateIsosInPath(path, XboxKrnlVersion->Build < 8008, isCci);
	
    if(isos.size() == 0) 
	{
		debug::print("Attach: No ISO's found.\r\n");
		HalReturnToFirmware(2);
	}

	if (XboxKrnlVersion->Build >= 8008) {
		debug::print("Detected Cerbios.\r\n");
		cerbiosAttach(path, isos, isCci);
	} else {
		debug::print("Cerbios Not Detected.\r\n");
		legacyAttach(path, isos, XboxKrnlVersion->Build);
	}
}


