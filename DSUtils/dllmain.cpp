#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

        break;
	case DLL_THREAD_ATTACH:
        break;
	case DLL_THREAD_DETACH:
        break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);
}