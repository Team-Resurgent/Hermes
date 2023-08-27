#include "iso_checker.h"
#include "debug.h"

BOOLEAN fileExists(PANSI_STRING file_path)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iostatusBlock;
    FILE_NETWORK_OPEN_INFORMATION fileInfo;
	HANDLE fileHandle = INVALID_HANDLE_VALUE;

	InitializeObjectAttributes(&objectAttributes, file_path, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = NtOpenFile(&fileHandle, GENERIC_READ | SYNCHRONIZE, &objectAttributes, &iostatusBlock, FILE_SHARE_READ, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
    if (!NT_SUCCESS(status) || (fileHandle==INVALID_HANDLE_VALUE)) {
		//debug::print("Failed To Open File %s\n", file_path->Buffer);
        return FALSE;
    }

	//debug::print("File Handle: %x\n", (int)fileHandle);

	memset(&fileInfo, 0, sizeof(fileInfo));

	status = NtQueryInformationFile(fileHandle,
        &iostatusBlock,
        &fileInfo,
        sizeof(fileInfo),
        FileNetworkOpenInformation);

	NtClose(fileHandle);

	if(!NT_SUCCESS(status)) {
		//debug::print("Failed To Query File %s\n", file_path->Buffer);
		return FALSE;
	}

	//debug::print("File Size: %d %d\n", fileInfo.EndOfFile.HighPart, fileInfo.EndOfFile.LowPart);

    return (fileInfo.EndOfFile.LowPart != 0);    
}

std::string lowerCase(std::string const& value)
{
	std::string result = std::string(value);
	std::transform(result.begin(), result.end(), result.begin(), tolower);
	return result;
}

bool endsWith(std::string const& value, std::string const& ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::vector<std::string> split(std::string const& value, char const& delimiter)
{
	std::string buffer;
	std::vector<std::string> items;
	for (size_t i = 0; i < value.size(); i++)
	{
		if (value.at(i) == delimiter)
		{
			items.push_back(buffer);
			buffer.clear();
			continue;
		}
		buffer.push_back(value.at(i));
	}
	if (!buffer.empty())
	{
		items.push_back(buffer);
	}
	return items;
}

std::vector<std::string> getIsoFiles(std::string const& path, bool isLegacy, bool &isCci)
{
	// Create a mount point so we can list files in folder

	char mountPoint[260];	
	sprintf(mountPoint, "\\??\\ISO:");

	char systemPath[260];
	sprintf(systemPath, "%s", path.c_str());

	STRING sMountPoint = { (USHORT)strlen(mountPoint), (USHORT)strlen(mountPoint) + 1, mountPoint };
	STRING sSystemPath = { (USHORT)strlen(systemPath), (USHORT)strlen(systemPath) + 1, systemPath };
	int result = IoCreateSymbolicLink(&sMountPoint, &sSystemPath);
	if (result != 0) {
		debug::print("Unable To Mount 'ISO:' Drive.");
		return std::vector<std::string>();
	}

	WIN32_FIND_DATA findData;
	memset(&findData, 0, sizeof(findData));
	std::string npath2 = "ISO:\\*";
	HANDLE findHandle = FindFirstFile(npath2.c_str(), &findData);
	if (findHandle == INVALID_HANDLE_VALUE) {
		debug::print("FindFirstFile in 'ISO:' Failed.");
		return std::vector<std::string>();
	}
	
	std::vector<std::string> isoFiles;

	int hasIsoCount = 0;
	int hasCciCount = 0;

	do {
		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			continue;	
		}
	
		std::string filename = lowerCase(findData.cFileName);
		
		bool hasIso = endsWith(filename, ".iso");
		if (hasIso == true)
		{
			hasIsoCount++;
		}

		bool hasCci = endsWith(filename, ".cci");
		if (hasCci == true)
		{
			isCci = true;
			hasCciCount++;
			if (isLegacy == true) {
				continue;
			}
		}

		if (hasIso == false && hasCci == false)
		{
			continue;
		}

		debug::print("Found ISO File '%s'\n", findData.cFileName);
		isoFiles.push_back(findData.cFileName);
	} 
	while (FindNextFile(findHandle, &findData) != 0);

	FindClose(findHandle);

	// Clean up

	char MountPoint[16];
	sprintf(MountPoint, "\\??\\ISO:");
	IoDeleteSymbolicLink(&sMountPoint);

	// Dont allow a mix of cci / iso
	if (hasIsoCount > 0 && hasCciCount > 0)
	{
		isoFiles.empty();
	}

	return isoFiles;
}

bool validateIsoArray(std::vector<std::string> const& isos)
{
	int count = isos.size();
	if (count == 0) {
		debug::print("Validation Failed, No ISO's Found.\n");
		return false;
	}

	//1 iso means no naming variants to worry about
	if (count == 1) {
		debug::print("Validation Ok.\n");
		return true;
	}

	//validate matching part counts and lengths
	int firstPartCount = split(isos[0], '.').size();
	int firstPartLength = isos[0].length();
	for (int i = 1; i < count; i++) {
		int nextPartCount = split(isos[i], '.').size();
		int nextPartLength = isos[i].length();
		if (firstPartCount != nextPartCount) {
			debug::print("Validation Failed, Mixed Naming Parts Found.\n");
			return false;
		}
		if (firstPartLength != nextPartLength) {
			debug::print("Validation Failed, Mixed Naming Lengths Found.\n");
			return false;
		}
	}

	//validate numbering
	for (int i = 0; i < count; i++) {
		std::vector<std::string> fileParts = split(isos[i], '.');
		if (fileParts.size() < 2) {
			debug::print("Validation Failed, Naming Issue.\n");
			return false;
		}
		std::string lastPart = fileParts[fileParts.size() - 2];
		if (lastPart.length() == 0 || !isdigit(lastPart[0])) {
			debug::print("Validation Failed, Slice Number Missing.\n");
			return false;
		}	
	}

	//Validate naming
	std::vector<std::string> firstParts = split(isos[0], '.');
	for (int i = 1; i < count; i++) {
		std::vector<std::string> nextParts = split(isos[i], '.');
		for (int j = 0; j < (int)firstParts.size() - 2; j++) {
			if (strncmp(lowerCase(firstParts[j]).c_str(), lowerCase(nextParts[j]).c_str(), firstParts[j].length()) != 0) {
				debug::print("Validation Failed, Naming Mismatch.\n");
				return false;
			}
		}
	}

	debug::print("Validation Ok.\n");
	return true;
}

void sortIsoArray(std::vector<std::string> isos)
{
	//sort array
	bool swapped = true;
	while (swapped) {
		swapped = false;
		for (int i = 0; i < (int)isos.size() - 1; i++) {       
			if(strcmp(isos[i].c_str(), isos[i+1].c_str()) > 0) {
				std::string temp = isos[i];
				isos.at(i) = isos[i + 1];
				isos[i + 1] = temp;
				swapped = true;
			}
		}
	}

	debug::print("ISO's Sorted...\n");
	for (int i = 0; i < (int)isos.size(); i++) { 
		debug::print("ISO %i = '%s'\n", i + 1, isos[i].c_str());
	}
}

std::string iso_checker::getPath()
{
	char path[MAX_PATHNAME];
    int pathlen;

    memset(path, 0, MAX_PATHNAME);
    strncpy(path, XeImageFileName->Buffer, XeImageFileName->Length < (MAX_PATHNAME - 1) ? XeImageFileName->Length : (MAX_PATHNAME - 1));
    pathlen = strrchr(path, '\\') - path;
    path[pathlen] = '\0';

	return std::string(path);
}

std::vector<std::string> iso_checker::validateIsosInPath(std::string path, bool isLegacy, bool &isCci)
{
	std::vector<std::string> isos = getIsoFiles(path, isLegacy, isCci);
	if (!validateIsoArray(isos)) {
		debug::print("Rebooting Due To Failed Validation.\n");
		HalReturnToFirmware(2); 
	}
	sortIsoArray(isos);

	debug::print("Attach: ISO Path %s\r\n", path.c_str());

	return isos;
}