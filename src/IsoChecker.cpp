#include "IsoChecker.h"
#include "debug.h"
#include "external.h"
#include "Defines.h"
#include "Path.h"
#include "StringUtility.h"

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
		Debug::Print("Unable To Mount 'ISO:' Drive.");
		return std::vector<std::string>();
	}

	WIN32_FIND_DATA findData;
	memset(&findData, 0, sizeof(findData));
	std::string npath2 = "ISO:\\*";
	HANDLE findHandle = FindFirstFile(npath2.c_str(), &findData);
	if (findHandle == INVALID_HANDLE_VALUE) {
		Debug::Print("FindFirstFile in 'ISO:' Failed.");
		return std::vector<std::string>();
	}
	
	std::vector<std::string> isoFiles;

	int hasIsoCount = 0;
	int hasCsoCount = 0;
	int hasCciCount = 0;

	do {
		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			continue;	
		}
	
		std::string filename = StringUtility::LowerCase(findData.cFileName);
		
		bool hasIso = StringUtility::EndsWith(filename, ".iso");
		if (hasIso == true)
		{
			hasIsoCount = 1;
		}

		bool hasCci = StringUtility::EndsWith(filename, ".cci");
		if (hasCci == true)
		{
			isCci = true;
			hasCciCount = 1;
			if (isLegacy == true) {
				continue;
			}
		}

		bool hasCso = StringUtility::EndsWith(filename, ".cso");
		if (hasCso == true)
		{
			hasCsoCount = 1;
			if (isLegacy == false) {
				continue;
			}
		}

		if (hasIso == false && hasCci == false && hasCso == false)
		{
			continue;
		}

		Debug::Print("Found ISO File '%s'\n", findData.cFileName);
		isoFiles.push_back(findData.cFileName);
	} 
	while (FindNextFile(findHandle, &findData) != 0);

	FindClose(findHandle);

	// Clean up

	char MountPoint[16];
	sprintf(MountPoint, "\\??\\ISO:");
	IoDeleteSymbolicLink(&sMountPoint);

	// Dont allow a mix of cci / iso
	if ((hasIsoCount + hasCciCount + hasCsoCount) > 1)
	{
		isoFiles.empty();
	}

	return isoFiles;
}

bool validateIsoArray(std::vector<std::string> const& isos)
{
	int count = isos.size();
	if (count == 0) {
		Debug::Print("Validation Failed, No ISO's Found.\n");
		return false;
	}

	//1 iso means no naming variants to worry about
	if (count == 1) {
		Debug::Print("Validation Ok.\n");
		return true;
	}

	//validate matching part counts and lengths
	int firstPartCount = StringUtility::Split(isos[0], '.').size();
	int firstPartLength = isos[0].length();
	for (int i = 1; i < count; i++) {
		int nextPartCount = StringUtility::Split(isos[i], '.').size();
		int nextPartLength = isos[i].length();
		if (firstPartCount != nextPartCount) {
			Debug::Print("Validation Failed, Mixed Naming Parts Found.\n");
			return false;
		}
		if (firstPartLength != nextPartLength) {
			Debug::Print("Validation Failed, Mixed Naming Lengths Found.\n");
			return false;
		}
	}

	//validate numbering
	for (int i = 0; i < count; i++) {
		std::vector<std::string> fileParts = StringUtility::Split(isos[i], '.');
		if (fileParts.size() < 2) {
			Debug::Print("Validation Failed, Naming Issue.\n");
			return false;
		}
		std::string lastPart = fileParts[fileParts.size() - 2];
		if (lastPart.length() == 0 || !isdigit(lastPart[0])) {
			Debug::Print("Validation Failed, Slice Number Missing.\n");
			return false;
		}	
	}

	//Validate naming
	std::vector<std::string> firstParts = StringUtility::Split(isos[0], '.');
	for (int i = 1; i < count; i++) {
		std::vector<std::string> nextParts = StringUtility::Split(isos[i], '.');
		for (int j = 0; j < (int)firstParts.size() - 2; j++) {
            if (strncmp(StringUtility::LowerCase(firstParts[j]).c_str(), StringUtility::LowerCase(nextParts[j]).c_str(), firstParts[j].length()) != 0) {
				Debug::Print("Validation Failed, Naming Mismatch.\n");
				return false;
			}
		}
	}

	Debug::Print("Validation Ok.\n");
	return true;
}

void sortIsoArray(std::vector<std::string> &isos)
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

	Debug::Print("ISO's Sorted...\n");
	for (int i = 0; i < (int)isos.size(); i++) { 
		Debug::Print("ISO %i = '%s'\n", i + 1, isos[i].c_str());
	}
}

bool IsoChecker::ContainsCheat()
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK iostatusBlock;

    char patchPath[MAX_PATHNAME];
    sprintf(patchPath, "%s\\trainer.etm", Path::GetAppPath().c_str());
	STRING sPatchPath = { (USHORT)strlen(patchPath), (USHORT)strlen(patchPath) + 1, patchPath};
    objectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
    objectAttributes.ObjectName = &sPatchPath;
    objectAttributes.RootDirectory = 0;

    status = NtOpenFile(&fileHandle, GENERIC_READ | SYNCHRONIZE, &objectAttributes, &iostatusBlock, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);
    if (status != 0) 
    {
        return false;
    }
    NtClose(fileHandle);

    return true;
}

std::vector<std::string> IsoChecker::ValidateIsosInPath(std::string path, bool isLegacy, bool &isCci)
{
	std::vector<std::string> isos = getIsoFiles(path, isLegacy, isCci);
	if (!validateIsoArray(isos)) {
		Debug::Print("Rebooting Due To Failed Validation.\n");
		HalReturnToFirmware(2); 
	}
	sortIsoArray(isos);

	Debug::Print("Attach: ISO Path %s\r\n", path.c_str());

	return isos;
}