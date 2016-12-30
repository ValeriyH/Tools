// MemoryScaner.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <Windows.h>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <list>
#include <vector>

union CAST
{
    DWORD dw;
    BYTE bt[4];
};

//TODO refactor code and remove global variables
HANDLE pHandle = NULL;
//TODO Search in memory not only dword, but string, long and etc.
CAST search = { 0 };
DWORD dwSearch = 0; //duplicate search.dw
std::list<BYTE*> addrs;

wchar_t* GetLastErrorAsString(DWORD error = GetLastError())
{
    static wchar_t buf[256];
    buf[0] = 0;
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
        MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), buf, sizeof(buf), NULL);
    return buf;
}

//Set process handle
bool OpenMyProcess(DWORD pID) 
{
    pHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | /*PROCESS_VM_WRITE | */PROCESS_QUERY_INFORMATION, FALSE, pID); //PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION
    if (pHandle == NULL) {
        printf("MemoryScanner:OpenProcess error: %ls\n", GetLastErrorAsString());
        return false;
    }

    return true;
}

//Input pHandle, output addrs list
void ScanMemmory() 
{
    SYSTEM_INFO sysInfo = { 0 };
    GetSystemInfo(&sysInfo);

    auto aStart = (long long)sysInfo.lpMinimumApplicationAddress;
    auto aEnd = (long long)sysInfo.lpMaximumApplicationAddress;

    //HANDLE pHandle = GetCurrentProcess(); //Use current process

    size_t buffer_size = 0;
    unsigned char* dump = NULL;
    while (aStart < aEnd) 
    {
        MEMORY_BASIC_INFORMATION mbi = { 0 };
        if (!VirtualQueryEx(pHandle, (LPCVOID)aStart, &mbi, sizeof(mbi))) 
        {
            printf("MemoryScanner:Read process memory error: %ls\n",  GetLastErrorAsString());
            return;
        }

        if (mbi.State == MEM_COMMIT && ((mbi.Protect & PAGE_GUARD) == 0) && ((mbi.Protect == PAGE_NOACCESS) == 0)) {

            auto isWritable = ((mbi.Protect & PAGE_READWRITE) != 0 || (mbi.Protect & PAGE_WRITECOPY) != 0 || (mbi.Protect & PAGE_EXECUTE_READWRITE) != 0 || (mbi.Protect & PAGE_EXECUTE_WRITECOPY) != 0);
            if (isWritable) {

                //TODO it search in remoute process memory but retuns point in current process memory
                //auto dump = new unsigned char[mbi.RegionSize + 1];
                if (mbi.RegionSize + 1 > buffer_size)
                {
                    delete[] dump;
                    dump = new unsigned char[mbi.RegionSize + 1];
                    buffer_size = mbi.RegionSize + 1;
                }
                memset(dump, 0x00, mbi.RegionSize + 1);
                ReadProcessMemory(pHandle, mbi.BaseAddress, dump, mbi.RegionSize, NULL);

                BYTE* block_start = dump;
                BYTE* block_end = dump + mbi.RegionSize;
                BYTE* token_start = (BYTE*)&dwSearch;
                BYTE* token_end = (BYTE*)&dwSearch + sizeof(dwSearch);
                BYTE* found = std::search(block_start, block_end, token_start, token_end);
                    
                for (;found < block_end; found = std::search(++found, block_end, token_start, token_end))
                {
                    BYTE* addr = (BYTE*)mbi.BaseAddress + (found - block_start);
                    //printf("Found at 0x%p\n", (LPVOID)addr);
                    addrs.push_back(addr);
                }


            }
        }
        aStart += mbi.RegionSize;
    }
    delete[] dump;
}



template <class InIter1, class InIter2, class OutIter>
void find_all(unsigned char *base, InIter1 buf_start, InIter1 buf_end, InIter2 pat_start, InIter2 pat_end, OutIter res) {
    for (InIter1 pos = buf_start;
        buf_end != (pos = std::search(pos, buf_end, pat_start, pat_end));
        ++pos)
    {
        //*res++ = base + (pos - buf_start);

        BYTE* addr = base + (pos - buf_start);
        addrs.push_back(addr);
    }
}

template <class outIter>
void find_locs(HANDLE process, std::vector<BYTE> const &pattern, outIter output) {

    unsigned char *p = NULL;
    MEMORY_BASIC_INFORMATION info;

    for (p = NULL;
        VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info);
        p += info.RegionSize)
    {
        std::vector<char> buffer;
        std::vector<char>::iterator pos;

        if (info.State == MEM_COMMIT &&
            (info.Type == MEM_MAPPED || info.Type == MEM_PRIVATE))
        {
            SIZE_T bytes_read;
            buffer.resize(info.RegionSize);
            ReadProcessMemory(process, p, &buffer[0], info.RegionSize, &bytes_read);
            buffer.resize(bytes_read);
            find_all(p, buffer.begin(), buffer.end(), pattern.begin(), pattern.end(), output);
        }
    }
}

void MemoryCorrection(bool changed)
{
    printf("Searching %s values\n", changed ? "changed" : "unchanged");
    DWORD data;
    std::list<BYTE*> persist;
    for each (auto addr in addrs)
    {
        if (ReadProcessMemory(pHandle, addr, &data, sizeof(data), NULL))
        {
            if (!changed && data == dwSearch)
            {
                persist.push_back(addr);
            }
            if (changed && data != dwSearch)
            {
                persist.push_back(addr);
            }
        }
    }
    addrs.swap(persist);
}

void ShowList()
{
    DWORD data;
    for each (auto addr in addrs)
    {
        if (ReadProcessMemory(pHandle, addr, &data, sizeof(data), NULL))
        {
            printf("Addr 0x%p Data %d\n", (LPVOID)addr, data);
        }
        else
        {
            printf("Addr 0x%p Unreadable\n", (LPVOID)addr);
        }
    }
}

void InjectDll(DWORD procID)
{
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer); //TODO use Exe file location GetModuleFileName( NULL, buffer, MAX_PATH ); and truncate exe name. or by argv[0]
    std::string path = buffer;
    path += "\\inject.dll";

    const char* dll_path = path.c_str();
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
    //_setmode(_fileno(stdout), _O_U16TEXT);
    if (argc < 3)
    {
        printf("Usage: MemoryScaner <PID> <SEARCH DWORD>\n");
        return 1;
    }

    DWORD id = atoi(argv[1]);
    search.dw = dwSearch = atoi(argv[2]);
    
    printf("Opening process %d...\n", id);
    //InjectDll(id);
    if (!OpenMyProcess(id))
    {
        return 2;
    }

    std::vector<BYTE> vSearch(search.bt, search.bt + sizeof(search.bt)/sizeof(BYTE));

    printf("1st searching %d\n", search.dw);
    ScanMemmory();
    //find_locs(pHandle, vSearch, NULL);
    printf("Found %zd items\n", addrs.size());

    int key = 'n';
    do
    {
        printf("Updating list\n");
        MemoryCorrection(key == 'c');
        printf("Found %zd items\n", addrs.size());
        if (addrs.size() < 10)
        {
            break;
        }
        printf("Press space to finish, <c> if value changed other key if value not changed\n");
        key = getchar();
    } while (key != ' ');
 

    if (addrs.size() == 0)
    {
        return 3;
    }

    do
    {
        ShowList();
        Sleep(1000);
        puts("");
    } while (true);
   
    return 0;
}

