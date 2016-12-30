#include "stdafx.h"
#include "Injector.h"
#include <string>

//TODO Remove this global function
extern wchar_t* GetLastErrorAsString(DWORD error = GetLastError());

Injector::Injector()
{
}


Injector::~Injector()
{
}


//TODO: Refactor:
//#1 Dynamically load Inject32.dll/Inject64.dll from application folder.
//#2 Improve error handling. Throw exception or return error code
//#3 Do not use printf inside function
//NOTE: injected.dll should be x32 or x64. Base on process type (as for now x64 version doesn't work)
//NOTE: Antivirus can handle access and block create remote thread
void Injector::InjectDll(DWORD procID)
{
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer); //TODO use Exe file location GetModuleFileName( NULL, buffer, MAX_PATH ); and truncate exe name. or by argv[0]
    std::string path = buffer;
    path += "\\inject.dll";

    path = "d:\\Training\\Tools\\MemoryScaner\\Debug\\Inject.dll";
    //path = "d:\\Training\\Tools\\MemoryScaner\\x64\\Debug\\Inject.dll" ;

    const char* dll_path = path.c_str();
    //char* dll_path = "C:\\drivers\\dllinject.dll";
    /*
    * Get process handle passing in the process ID.
    */

    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    if (process == NULL) {
        printf("Error: the specified process couldn't be open. %ls\n", GetLastErrorAsString());
    }
    /*
    * Get address of the LoadLibrary function.
    */
    LPVOID addr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    if (addr == NULL) {
        printf("Error: the LoadLibraryA function was not found inside kernel32.dll library. %ls\n", GetLastErrorAsString());
    }
    /*
    * Allocate new memory region inside the process's address space.
    */
    LPVOID arg = (LPVOID)VirtualAllocEx(process, NULL, strlen(dll_path), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (arg == NULL) {
        printf("Error: the memory could not be allocated inside the chosen process. %ls\n", GetLastErrorAsString());
    }
    /*
    * Write the argument to LoadLibraryA to the process's newly allocated memory region.
    */
    int n = WriteProcessMemory(process, arg, dll_path, strlen(dll_path), NULL);
    if (n == 0) {
        printf("Error: there was no bytes written to the process's address space. %ls\n", GetLastErrorAsString());
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
}