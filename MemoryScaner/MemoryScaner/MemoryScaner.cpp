// MemoryScaner.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <algorithm>
#include <stdio.h>

HANDLE pHandle = NULL;
HANDLE threadScan = NULL;
DWORD threadId = 0;
bool runThread = false;
LPWSTR find_str = L"Hello";


wchar_t* GetLastErrorAsString(DWORD error = GetLastError())
{
    static wchar_t buf[256];
    buf[0] = 0;
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, sizeof(buf), NULL);
    return buf;
}

void ScanMemmory() {

    std::wstring str = find_str; //static resources not scaned

    SYSTEM_INFO sysInfo = { 0 };
    GetSystemInfo(&sysInfo);

    auto aStart = (long)sysInfo.lpMinimumApplicationAddress;
    auto aEnd = (long)sysInfo.lpMaximumApplicationAddress;

    int found = 0;

    //HANDLE pHandle = GetCurrentProcess(); //Use current process

    do {

        while (aStart < aEnd) {
            MEMORY_BASIC_INFORMATION mbi = { 0 };
            if (!VirtualQueryEx(pHandle, (LPCVOID)aStart, &mbi, sizeof(mbi))) {
                printf("MemoryScanner:Read process memory error: %ls",  GetLastErrorAsString());
                CloseHandle(pHandle);
                TerminateThread(threadScan, 1);
            }

            if (mbi.State == MEM_COMMIT && ((mbi.Protect & PAGE_GUARD) == 0) && ((mbi.Protect == PAGE_NOACCESS) == 0)) {

                auto isWritable = ((mbi.Protect & PAGE_READWRITE) != 0 || (mbi.Protect & PAGE_WRITECOPY) != 0 || (mbi.Protect & PAGE_EXECUTE_READWRITE) != 0 || (mbi.Protect & PAGE_EXECUTE_WRITECOPY) != 0);
                if (isWritable) {

                    //TODO it search in remoute process memory but retuns point in current process memory
                    auto dump = new unsigned char[mbi.RegionSize + 1];
                    memset(dump, 0x00, mbi.RegionSize + 1);
                    ReadProcessMemory(pHandle, mbi.BaseAddress, dump, mbi.RegionSize, NULL);

                    unsigned char* block_start = dump;
                    unsigned char* block_end = dump + mbi.RegionSize;
                    unsigned char* token_start = (unsigned char*)find_str;
                    unsigned char* token_end = (unsigned char*)find_str + 10;
                    unsigned char* found = std::search(block_start, block_end, token_start, token_end);
                    
                    for (;found < block_end; found = std::search(++found, block_end, token_start, token_end))
                    {
                        printf("Found at 0x%X\n", (unsigned char*)mbi.BaseAddress + (found - block_start));
                    }

                    delete[] dump;
                }

            }
            aStart += mbi.RegionSize;
        }
        runThread = false;

    } while (runThread);

    if (!runThread) {
        CloseHandle(pHandle);
        TerminateThread(threadScan, 0);
    }
}

bool OpenMyProcess(DWORD pID) {
    pHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | /*PROCESS_VM_WRITE | */PROCESS_QUERY_INFORMATION, FALSE, pID); //PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION
    if (pHandle == NULL) {
        printf("MemoryScanner:OpenProcess error: %ls", GetLastErrorAsString());
        return false;
    }

    if (runThread) {
        threadScan = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ScanMemmory, NULL, 0, &threadId);
    }
    else
    {
        ScanMemmory();
    }

    return true;
}


void InjectDll(DWORD procID, const char* dll_path)
{
    //char* dll_path = "C:\\drivers\\dllinject.dll";
    /*
    * Get process handle passing in the process ID.
    */

    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    if (process == NULL) {
        printf("Error: the specified process couldn't be found.\n");
    }
    /*
    * Get address of the LoadLibrary function.
    */
    LPVOID addr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    if (addr == NULL) {
        printf("Error: the LoadLibraryA function was not found inside kernel32.dll library.\n");
    }
    /*
    * Allocate new memory region inside the process's address space.
    */
    LPVOID arg = (LPVOID)VirtualAllocEx(process, NULL, strlen(dll_path), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (arg == NULL) {
        printf("Error: the memory could not be allocated inside the chosen process.\n");
    }
    /*
    * Write the argument to LoadLibraryA to the process's newly allocated memory region.
    */
    int n = WriteProcessMemory(process, arg, dll_path, strlen(dll_path), NULL);
    if (n == 0) {
        printf("Error: there was no bytes written to the process's address space.\n");
    }
    /*
    * Inject our DLL into the process's address space.
    */
    HANDLE threadID = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, NULL, NULL);
    if (threadID == NULL) {
        printf("Error: the remote thread could not be created. %ls\n", GetLastErrorAsString());
    }
    else {
        printf("Success: the remote thread was successfully created.\n");
    }
    /*
    * Close the handle to the process, becuase we've already injected the DLL.
    */
    CloseHandle(process);
    getchar();
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: MemoryScaner <PID>\n");
        return 1;
    }
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer); //TODO use Exe file location GetModuleFileName( NULL, buffer, MAX_PATH ); and truncate exe name. or by argv[0]
    std::string path = buffer;
    path += "\\inject.dll";


    DWORD id = atoi(argv[1]);
    printf("Opening process %d...\n", id);
    //InjectDll(id, path.c_str());
    OpenMyProcess(id);
    return 0;
}

