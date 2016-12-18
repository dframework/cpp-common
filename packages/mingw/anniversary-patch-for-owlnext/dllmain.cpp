#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//
// Defines undocumented Win32 function DeviceCapabilitiesExW, missing from GDI32.DLL in Windows 10 Version 1607.
// The GDI32.DLL forwards DeviceCapabilitiesExW to 'ext-ms-win-gdi-desktop-l1-1-0.DeviceCapabilitiesExA'.
// Note the trailing 'A'. This is probably a bug. We deal with it by using an alias in the module definition
// file (see "*.def" project files).
//
extern "C" int WINAPI DeviceCapabilitiesExW(LPCWSTR, LPCWSTR, LPCWSTR, WORD, LPWSTR, CONST DEVMODEW*)
{
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return -1;
}

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID)
{
  return TRUE;
}

