#pragma once

#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include "external.h"

#define STATUS_SUCCESS 0
#define STATUS_UNSUCCESSFUL -1
typedef ULONG NTSTATUS;

#define NT_SUCCESS(StatCode)  ((NTSTATUS)(StatCode) >= 0)

extern "C" {

	typedef ULONG ACCESS_MASK;
	typedef struct _ANSI_STRING {
		WORD Length;
		WORD MaximumLength;
		PSTR Buffer;
	} ANSI_STRING, *PANSI_STRING, STRING, *PSTRING;

	typedef struct _OBJECT_ATTRIBUTES
	{
			HANDLE RootDirectory;
			PANSI_STRING ObjectName;
			ULONG Attributes;
	} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

	typedef struct _IO_STATUS_BLOCK
	{
			NTSTATUS Status;
			ULONG Information;
	} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;


	// ******************************************************************
	// * FILE_INFORMATION_CLASS
	// ******************************************************************
	typedef enum _FILE_INFORMATION_CLASS
	{
		FileDirectoryInformation        = 1,
		FileFullDirectoryInformation,
		FileBothDirectoryInformation,
		FileBasicInformation,
		FileStandardInformation,
		FileInternalInformation,
		FileEaInformation,
		FileAccessInformation,
		FileNameInformation,
		FileRenameInformation,
		FileLinkInformation,
		FileNamesInformation,
		FileDispositionInformation,
		FilePositionInformation,
		FileFullEaInformation,
		FileModeInformation,
		FileAlignmentInformation,
		FileAllInformation,
		FileAllocationInformation,
		FileEndOfFileInformation,
		FileAlternateNameInformation,
		FileStreamInformation,
		FilePipeInformation,
		FilePipeLocalInformation,
		FilePipeRemoteInformation,
		FileMailslotQueryInformation,
		FileMailslotSetInformation,
		FileCompressionInformation,
		FileCopyOnWriteInformation,
		FileCompletionInformation,
		FileMoveClusterInformation,
		FileQuotaInformation,
		FileReparsePointInformation,
		FileNetworkOpenInformation,
		FileObjectIdInformation,
		FileTrackingInformation,
		FileOleDirectoryInformation,
		FileContentIndexInformation,
		FileInheritContentIndexInformation,
		FileOleInformation,
		FileMaximumInformation
	} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

	typedef struct _FILE_NETWORK_OPEN_INFORMATION {
		LARGE_INTEGER CreationTime;
		LARGE_INTEGER LastAccessTime;
		LARGE_INTEGER LastWriteTime;
		LARGE_INTEGER ChangeTime;
		LARGE_INTEGER AllocationSize;
		LARGE_INTEGER EndOfFile;
		ULONG FileAttributes;
	} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;

	typedef struct _FILE_DIRECTORY_INFORMATION {
		ULONG NextEntryOffset;
		ULONG FileIndex;
		LARGE_INTEGER CreationTime;
		LARGE_INTEGER LastAccessTime;
		LARGE_INTEGER LastWriteTime;
		LARGE_INTEGER ChangeTime;
		LARGE_INTEGER EndOfFile;
		LARGE_INTEGER AllocationSize;
		ULONG FileAttributes;
		ULONG FileNameLength;
		CHAR FileName[1];
	} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

	typedef struct _FILE_FS_SIZE_INFORMATION {
		LARGE_INTEGER TotalAllocationUnits;
		LARGE_INTEGER AvailableAllocationUnits;
		ULONG SectorsPerAllocationUnit;
		ULONG BytesPerSector;
	} FILE_FS_SIZE_INFORMATION, *PFILE_FS_SIZE_INFORMATION;

	typedef struct _DISK_GEOMETRY {
		LARGE_INTEGER Cylinders;
		DWORD MediaType;
		DWORD TracksPerCylinder;
		DWORD SectorsPerTrack;
		DWORD BytesPerSector;
	} DISK_GEOMETRY, *PDISK_GEOMETRY;

	extern NTSTATUS NtQueryFullAttributesFile
	(
		IN POBJECT_ATTRIBUTES ObjectAttributes,
		OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation
	);

	typedef VOID (*PIO_APC_ROUTINE)(
		PVOID ApcContext,
		PIO_STATUS_BLOCK pIoStatusBlock,
		ULONG Reserved
	);

	extern NTSTATUS WINAPI NtDeviceIoControlFile(
		HANDLE handle,
		HANDLE event,
		PVOID pApcRoutine,
		PVOID pApcContext,
		PIO_STATUS_BLOCK pIoStatusBlock,
		ULONG IoControlCode,
		PVOID pInBuf,
		ULONG InBufLen,
		PVOID pOutBuf,
		ULONG OutBufLen
	);

	extern NTSTATUS WINAPI NtCreateFile(
		OUT PHANDLE FileHandle,
		IN ACCESS_MASK DesiredAccess,
		IN POBJECT_ATTRIBUTES ObjectAttributes,
		OUT PIO_STATUS_BLOCK IoStatusBlock,
		IN PLARGE_INTEGER AllocationSize OPTIONAL,
		IN ULONG FileAttributes,
		IN ULONG ShareAccess,
		IN ULONG CreateDisposition,
		IN ULONG CreateOptions
	);

	#define InitializeObjectAttributes(p, n, a, r, s) { \
		(p)->RootDirectory = r; \
		(p)->Attributes = a; \
		(p)->ObjectName = n; \
	}

	#define ObDosDevicesDirectory() ((HANDLE)-3)
	#define ObWin32NamedObjectsDirectory() ((HANDLE)-4)

	extern NTSTATUS WINAPI NtOpenFile(
		OUT PHANDLE pHandle,
		IN ACCESS_MASK DesiredAccess,
		IN POBJECT_ATTRIBUTES pObjectAttributes,
		OUT PIO_STATUS_BLOCK pIOStatusBlock,
		IN ULONG ShareAccess,
		IN ULONG OpenOptions
	);

	extern NTSTATUS WINAPI NtReadFile(
		HANDLE Handle,
		HANDLE Event,
		PVOID pApcRoutine,
		PVOID pApcContext,
		PVOID pIoStatusBlock,
		PVOID pBuffer,
		ULONG Length,
		PLARGE_INTEGER pByteOffset
	);

	extern NTSTATUS WINAPI NtWriteFile(
		HANDLE Handle,
		HANDLE Event,
		PVOID pApcRoutine,
		PVOID pApcContext,
		PVOID pIoStatusBlock,
		PVOID pBuffer,
		ULONG Length,
		PLARGE_INTEGER pByteOffset
	);

	extern NTSTATUS WINAPI NtClose(HANDLE Handle);

	extern NTSTATUS WINAPI NtQueryInformationFile(
		HANDLE FileHandle,
		PIO_STATUS_BLOCK IoStatusBlock,
		PVOID FileInformation,
		ULONG FileInformationLength,
		FILE_INFORMATION_CLASS FileInformationClass
	);

	extern NTSTATUS WINAPI NtQueryDirectoryFile(
		HANDLE	Handle,
		HANDLE	Event,
		PIO_APC_ROUTINE ApcRoutine,
		PVOID	ApcContext,
		PIO_STATUS_BLOCK IoStatusBlock,
		PVOID	FileInformation,
		ULONG	Length,
		ULONG	FileInformationClass,
		PANSI_STRING FileName,
		BOOLEAN RestartScan
	);

	extern NTSTATUS WINAPI NtAllocateVirtualMemory(PVOID*, ULONG, PULONG, ULONG, ULONG);
	extern VOID WINAPI HalReturnToFirmware(unsigned int);
	extern VOID WINAPI RtlInitAnsiString(void *destp, void *srcp);

	extern LONG WINAPI RtlCompareString(
		PSTRING String1,
		PSTRING String2,
		BOOLEAN CaseInSensitive
	);
	extern PANSI_STRING XeImageFileName;

	extern NTSTATUS WINAPI IoDismountVolumeByName(PANSI_STRING DeviceName);

	struct FileInfo {
		FILE_DIRECTORY_INFORMATION dirInfo;
		// Reserve path buffer minus the null-terminator and the first byte provided by dirInfo
		char filename[MAX_PATH-2];
	};

	// CreateDisposition values for NtCreateFile()
	#define FILE_SUPERSEDE                          0x00000000
	#define FILE_OPEN                               0x00000001
	#define FILE_CREATE                             0x00000002
	#define FILE_OPEN_IF                            0x00000003
	#define FILE_OVERWRITE                          0x00000004
	#define FILE_OVERWRITE_IF                       0x00000005
	#define FILE_MAXIMUM_DISPOSITION                0x00000005

	// CreateOption values for NtCreateFile()
	// FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT is what CreateFile
	// uses for most things when translating to NtCreateFile.
	#define FILE_DIRECTORY_FILE                     0x00000001
	#define FILE_WRITE_THROUGH                      0x00000002
	#define FILE_SEQUENTIAL_ONLY                    0x00000004
	#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008
	#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
	#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
	#define FILE_NON_DIRECTORY_FILE                 0x00000040
	#define FILE_CREATE_TREE_CONNECTION             0x00000080
	#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
	#define FILE_NO_EA_KNOWLEDGE                    0x00000200
	#define FILE_OPEN_FOR_RECOVERY                  0x00000400
	#define FILE_RANDOM_ACCESS                      0x00000800
	#define FILE_DELETE_ON_CLOSE                    0x00001000
	#define FILE_OPEN_BY_FILE_ID                    0x00002000
	#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
	#define FILE_NO_COMPRESSION                     0x00008000
	#define FILE_RESERVE_OPFILTER                   0x00100000
	#define FILE_OPEN_REPARSE_POINT                 0x00200000
	#define FILE_OPEN_NO_RECALL                     0x00400000
	#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000
	#define FILE_COPY_STRUCTURED_STORAGE            0x00000041
	#define FILE_STRUCTURED_STORAGE                 0x00000441
	#define FILE_VALID_OPTION_FLAGS                 0x00ffffff
	#define FILE_VALID_PIPE_OPTION_FLAGS            0x00000032
	#define FILE_VALID_MAILSLOT_OPTION_FLAGS        0x00000032
	#define FILE_VALID_SET_FLAGS                    0x00000036

	// Access flags
	#define FILE_SHARE_READ                         0x00000001
	#define FILE_SHARE_WRITE                        0x00000002
	#define FILE_SHARE_DELETE                       0x00000004
	#define FILE_ATTRIBUTE_NORMAL                   0x00000080
	#define FILE_FLAG_RANDOM_ACCESS                 0x10000000

	#define OBJ_CASE_INSENSITIVE		0x00000040L
	#define STATUS_NO_MORE_FILES		0x80000006

	#define IOCTL_VIRTUAL_ATTACH	0xCE52B01
	#define IOCTL_VIRTUAL_DETACH	0xCE52B02

	#define IOCTL_VIRTUAL_CDROM_ATTACH	0x1EE7CD01
	#define IOCTL_VIRTUAL_CDROM_DETACH	0x1EE7CD02

	#define MAX_IMAGE_SLICES			8

	typedef struct _ATTACH_SLICE_DATA_CERBIOS {
		UCHAR SliceCount;
		UCHAR DeviceType;
		UCHAR Reserved1;
		UCHAR Reserved2;
		ANSI_STRING SliceFile[MAX_IMAGE_SLICES];
		ANSI_STRING MountPoint;
	} ATTACH_SLICE_DATA_CERBIOS;

	typedef struct _ATTACH_SLICE_DATA_LEGACY {
		unsigned int SliceCount;
		ANSI_STRING SliceFile[MAX_IMAGE_SLICES];
	} ATTACH_SLICE_DATA_LEGACY;

	#define MAX_PATHNAME		256

	LONG WINAPI IoCreateSymbolicLink(STRING*, STRING*);
	LONG WINAPI IoDeleteSymbolicLink(STRING*);

	typedef struct _XBOX_KRNL_VERSION {
		USHORT Major;
		USHORT Minor;
		USHORT Build;
		USHORT Qfe;
	} XBOX_KRNL_VERSION, *PXBOX_KRNL_VERSION;

	extern const PXBOX_KRNL_VERSION XboxKrnlVersion;
}