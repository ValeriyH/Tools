#include "stdafx.h"
#include "MemoryMonitor.h"
#include <Psapi.h>
#include <algorithm>


//TODO Refactor
//#1 Remove printf.
//#2 Handle error in proper way (exception or ret code)
//#3 Refactor. Internal function like _OpenProcess can throw exception. Interface function should handle this exception and return error code.
//#4 Add memory dump (see Sandbox)
//#5 Reuse BlockScan in FullScan

//TODO Remove this global function
extern wchar_t* GetLastErrorAsString(DWORD error = GetLastError());

void LogError(const char* msg, const char* file, int line)
{
    static wchar_t buf[256];
    buf[0] = 0;
    DWORD error = GetLastError();

    if (error != 0)
    {
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
            MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), buf, sizeof(buf), NULL);
    }
    printf("\nERROR: %s. %s. at %s:%d\n", msg, buf, file, line);
}
#define LOGERROR(msg) LogError(msg, __FILE__, __LINE__)

//TODO Integrate to class
HMODULE GetExeModule(HANDLE hProcess)
{
    HMODULE hMods[1024];
    HANDLE pHandle = hProcess;
    DWORD cbNeeded;
    unsigned int i;

    if (EnumProcessModules(pHandle, hMods, sizeof(hMods), &cbNeeded))
    {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];
            if (GetModuleFileNameEx(pHandle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
            {
                std::wstring wstrModName = szModName;
                //TODO it should end to .exe
                std::wstring wstrModContain = L".exe";
                if (wstrModName.find(wstrModContain) != std::wstring::npos)
                {
                    return hMods[i];
                }
            }
        }
    }
    return NULL;
}

MemoryMonitor::MemoryMonitor(int pid)
{
    _hProcess = NULL;
    _pid = pid;
}

MemoryMonitor::~MemoryMonitor()
{
    _CloseProcess();
}

void MemoryMonitor::ShowModules() 
{

    if (!_hProcess)
    {
        printf("Process not set");
        return;
    }

    size_t usage = 0;
    HANDLE process = _hProcess;
    unsigned char *p = NULL;
    MEMORY_BASIC_INFORMATION info;

    for (p = NULL;
        VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info);
        p += info.RegionSize)
    {
        printf("%#10.10x (%6uK)\t", info.BaseAddress, info.RegionSize / 1024);

        switch (info.State) {
        case MEM_COMMIT:
            printf("Committed");
            break;
        case MEM_RESERVE:
            printf("Reserved");
            break;
        case MEM_FREE:
            printf("Free");
            break;
        }
        printf("\t");
        switch (info.Type) {
        case MEM_IMAGE:
            printf("Code Module");
            break;
        case MEM_MAPPED:
            printf("Mapped     ");
            break;
        case MEM_PRIVATE:
            printf("Private    ");
        }
        printf("\t");

        if ((info.State == MEM_COMMIT) && (info.Type == MEM_PRIVATE))
            usage += info.RegionSize;

        int guard = 0, nocache = 0;

        if (info.AllocationProtect & PAGE_NOCACHE)
            nocache = 1;
        if (info.AllocationProtect & PAGE_GUARD)
            guard = 1;

        info.AllocationProtect &= ~(PAGE_GUARD | PAGE_NOCACHE);

        switch (info.AllocationProtect) {
        case PAGE_READONLY:
            printf("Read Only");
            break;
        case PAGE_READWRITE:
            printf("Read/Write");
            break;
        case PAGE_WRITECOPY:
            printf("Copy on Write");
            break;
        case PAGE_EXECUTE:
            printf("Execute only");
            break;
        case PAGE_EXECUTE_READ:
            printf("Execute/Read");
            break;
        case PAGE_EXECUTE_READWRITE:
            printf("Execute/Read/Write");
            break;
        case PAGE_EXECUTE_WRITECOPY:
            printf("COW Executable");
            break;
        }

        if (guard)
            printf("\tguard page");
        if (nocache)
            printf("\tnon-cachable");
        printf("\n");
    }
}

int MemoryMonitor::FullScan(DWORD* dwSearch, int count)
{
    if (!_OpenProcess())
    {
        return -1;
     }

    _ASSERT(_hProcess && "Handle should be initiated here!");
    _addrs.clear();

    HANDLE pHandle = _hProcess;
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
            printf("MemoryScanner:Read process memory error: %ls\n", GetLastErrorAsString());
            return -1;
        }

        //Ignore image pages, no access pages and guard pages
        //Ignore image pages: ((mbi.Type & MEM_IMAGE) == 0) 
        //Search only private
        if (mbi.State == MEM_COMMIT && (mbi.Type & MEM_PRIVATE) && ((mbi.Protect & PAGE_GUARD) == 0) && ((mbi.Protect & PAGE_NOACCESS) == 0)) {

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

                //BYTE* block_start = dump;
                //BYTE* block_end = dump + mbi.RegionSize;
                //BYTE* token_start = (BYTE*)&dwSearch;
                //BYTE* token_end = (BYTE*)&dwSearch + sizeof(dwSearch);
                //BYTE* found = std::search(block_start, block_end, token_start, token_end);

                //for (; found < block_end; found = std::search(++found, block_end, token_start, token_end))
                //{
                //    BYTE* addr = (BYTE*)mbi.BaseAddress + (found - block_start);
                //    //printf("Found at 0x%p\n", (LPVOID)addr);
                //    _addrs.push_back(addr);
                //}

                //Not universal but 4 time faster than BYTE*
                DWORD* block_start = (DWORD*)dump;
                DWORD* block_end = (DWORD*)(dump + mbi.RegionSize);
                DWORD* token_start = dwSearch;
                DWORD* token_end = dwSearch + count;
                DWORD* found = std::search(block_start, block_end, token_start, token_end);

                for (; found < block_end; found = std::search(++found, block_end, token_start, token_end))
                {
                    BYTE* addr = (BYTE*)mbi.BaseAddress + ((BYTE*)found - (BYTE*)block_start);
                    //printf("Found at 0x%p\n", (LPVOID)addr);
                    MemInfo info;
                    info.addr = addr;
                    info.base = (BYTE*)mbi.BaseAddress;
                    _addrs.push_back(info);
                }
            }
        }
        aStart += mbi.RegionSize;
    }
    delete[] dump;

    return _addrs.size();
}

int MemoryMonitor::BlockScan(DWORD value, PVOID start, size_t size)
{
    if (!_OpenProcess())
    {
        return -1;
    }

    _ASSERT(_hProcess && "Handle should be initiated here!");
    _addrs.clear();

    if (!size)
    {
        MEMORY_BASIC_INFORMATION mbi = { 0 };
        if (!VirtualQueryEx(_hProcess, (LPCVOID)start, &mbi, sizeof(mbi)))
        {
            printf("MemoryScanner:Read process memory error: %ls\n", GetLastErrorAsString());
            return -1;
        }
        start = mbi.BaseAddress;
        size = mbi.RegionSize;
    }

    DWORD dwSearch = value;
    BYTE* dump = new BYTE[size + 1];

    memset(dump, 0x00, size + 1);
    if (ReadProcessMemory(_hProcess, start, dump, size, NULL))
    {
        BYTE* block_start = dump;
        BYTE* block_end = dump + size;
        BYTE* token_start = (BYTE*)&dwSearch;
        BYTE* token_end = (BYTE*)&dwSearch + sizeof(dwSearch);
        while (block_start < block_end)
        {
            block_start = std::search(block_start, block_end, token_start, token_end);
            if (block_start == block_end)
            {
                break;
            }
            MemInfo info;
            info.addr = (BYTE*)start + ((BYTE*)block_start - (BYTE*)dump);
            info.base = (BYTE*)start;
            _addrs.push_back(info);
            block_start++;
        }
    }

    delete[] dump;
    return _addrs.size();
}

int MemoryMonitor::MemoryCorrection(DWORD value, bool equal)
{
    _ASSERT(_hProcess && "InitialScan should be called first!");
    DWORD data;
    std::list<MemInfo> persist;
    for each (auto addr in _addrs)
    {
        if (ReadProcessMemory(_hProcess, addr.addr, &data, sizeof(data), NULL))
        {
            if (equal && data == value)
            {
                persist.push_back(addr);
            }
            if (!equal && data != value)
            {
                persist.push_back(addr);
            }
        }
    }
    _addrs.swap(persist);
    return _addrs.size();
}

void MemoryMonitor::ShowList()
{
    _ASSERT(_hProcess && "InitialScan should be called first!");
    DWORD data[25];
    BYTE* appbase = NULL;
    HMODULE Module = GetExeModule(_hProcess);
    MODULEINFO info = { 0 };
    if (GetModuleInformation(_hProcess, Module, &info, sizeof(info)))
    {
        appbase = (BYTE*)info.lpBaseOfDll;
        printf("Application Base 0x%p\n", appbase);
    }
    else
    {
        LOGERROR("GetModuleInformation");
        return;
    }

    for each (auto addr in _addrs)
    {
        if (ReadProcessMemory(_hProcess, addr.addr, &data, sizeof(data), NULL))
        {
            printf("Addr 0x%p BlockBase 0x%p BaseOffset 0x%lx AppOffset 0x%lx\n", (LPVOID)addr.addr, addr.base, addr.addr - addr.base, addr.addr - appbase);
            for (int i = 0; i < sizeof(data) / sizeof(DWORD); i++)
            {
                printf("0x%x - %d\n", i * sizeof(DWORD), data[i]);
            }
            puts("");
        }
        else
        {
            printf("Addr 0x%p Unreadable\n", (LPVOID)addr.addr);
        }
    }
}

bool MemoryMonitor::_OpenProcess()
{
    _CloseProcess();
    _hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | /*PROCESS_VM_WRITE | */PROCESS_QUERY_INFORMATION, FALSE, _pid); //PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION
    if (_hProcess == NULL) {
        //TODO Throw exception 
        printf("MemoryScanner:OpenProcess error: %ls\n", GetLastErrorAsString());
        return false;
    }

    return true;
}

void MemoryMonitor::_CloseProcess()
{
    if (_hProcess)
    {
        CloseHandle(_hProcess);
        _hProcess = NULL;
    }
}
