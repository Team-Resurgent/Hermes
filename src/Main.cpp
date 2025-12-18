#include "IsoChecker.h"
#include "debug.h"
#include "external.h"
#include "Trainer.h"
#include "Path.h"
#include "IsoAttacher.h"

void __cdecl main()
{
    std::string path = Path::GetAppPath();
	Debug::Print("Kernel Version [%u.%u.%u.%u].\n", XboxKrnlVersion->Major, XboxKrnlVersion->Minor, XboxKrnlVersion->Build, XboxKrnlVersion->Qfe);
	Debug::Print("Application Path %s\n", path.c_str());

    if (IsoChecker::ContainsCheat() == true)
    {
        Trainer::LaunchTrainerMenuIfExists();
    }

	bool isCci = false;
	std::vector<std::string> isos = IsoChecker::ValidateIsosInPath(path, XboxKrnlVersion->Build < 8008, isCci);

    if(isos.size() == 0) 
	{
		Debug::Print("Attach: No ISO's found.\n");
		HalReturnToFirmware(2);
	}

	if (XboxKrnlVersion->Build >= 8008) {
		Debug::Print("Detected Cerbios.\n");
        IsoAttacher::AttachCerbios(path, isos, isCci);
	} else {
		Debug::Print("Cerbios Not Detected.\n");
        IsoAttacher::AttachLegacy(path, isos, XboxKrnlVersion->Build);
	}
}


