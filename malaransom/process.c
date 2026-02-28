#include <windows.h>
#include <tlhelp32.h>
#include <string.h>
#include "process.h"
#include "data.h"

DWORD GetProcId(const char* name)
{
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snap, &pe)) {
        do {
            if (!lstrcmpiA(pe.szExeFile, name)) {
                zumahandle = snap;
                CloseHandle(snap);
                return pe.th32ProcessID;
            }
        } while (Process32Next(snap, &pe));
    }

    CloseHandle(snap);
    return 0;
}
