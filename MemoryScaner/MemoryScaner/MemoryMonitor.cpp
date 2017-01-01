#include "stdafx.h"
#include "MemoryMonitor.h"
#include <algorithm>

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

//TODO Refactor
//#1 Remove printf.
//#2 Handle error in proper way (exception or ret code)

MemoryMonitor::MemoryMonitor()
{
    _hProcess = NULL;
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

int MemoryMonitor::InitialScan(int pid, DWORD dwSearch)
{
    if (!_OpenProcess(pid))
    {
        return -1;
     }

    //TODO Remove from here
    //ShowModules();

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
                DWORD* token_start = &dwSearch;
                DWORD* token_end = &dwSearch + 1;
                DWORD* found = std::search(block_start, block_end, token_start, token_end);

                for (; found < block_end; found = std::search(++found, block_end, token_start, token_end))
                {
                    BYTE* addr = (BYTE*)mbi.BaseAddress + ((BYTE*)found - (BYTE*)block_start);
                    //printf("Found at 0x%p\n", (LPVOID)addr);
                    _addrs.push_back((BYTE*)addr);
                }
            }
        }
        aStart += mbi.RegionSize;
    }
    delete[] dump;

    return _addrs.size();
}

int MemoryMonitor::MemoryCorrection(DWORD value, bool equal)
{
    _ASSERT(_hProcess && "InitialScan should be called first!");
    DWORD data;
    std::list<BYTE*> persist;
    for each (auto addr in _addrs)
    {
        if (ReadProcessMemory(_hProcess, addr, &data, sizeof(data), NULL))
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
    DWORD data;
    for each (auto addr in _addrs)
    {
        if (ReadProcessMemory(_hProcess, addr, &data, sizeof(data), NULL))
        {
            printf("Addr 0x%p Data %d\n", (LPVOID)addr, data);
            //TODO this is debug
            MEMORY_BASIC_INFORMATION mbi = { 0 };
            if (VirtualQueryEx(_hProcess, (LPCVOID)addr, &mbi, sizeof(mbi)))
            {
                printf("AllocationBase %p \t BaseAddress %p\n", mbi.AllocationBase, mbi.BaseAddress);
                
                char* dump = new char[mbi.RegionSize + 1];
                memset(dump, 0x00, mbi.RegionSize + 1);
                std::string file_name("dump_");
                _itoa_s((int)mbi.BaseAddress, dump, MAX_PATH,  16);
                file_name += dump;
                file_name += ".bin";

                FILE* file = NULL;
                fopen_s(&file, file_name.c_str(), "r");
                if (file == NULL)
                {
                    //Create if file not exists
                    fopen_s(&file, file_name.c_str(), "w");
                    if (file)
                    {
                        if (ReadProcessMemory(_hProcess, mbi.BaseAddress, dump, mbi.RegionSize, NULL))
                        {
                            fwrite(dump, sizeof(char), mbi.RegionSize, file);
                        }
                        else
                        {
                            LOGERROR("ReadProcessMemory");
                        }
                        fclose(file);
                    }
                    else
                    {
                        LOGERROR("File not created");
                    }
                }
                
                delete[] dump;
                
            }
            puts("");
        }
        else
        {
            printf("Addr 0x%p Unreadable\n", (LPVOID)addr);
        }
    }
}

bool MemoryMonitor::_OpenProcess(int pid)
{
    _CloseProcess();
    _hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | /*PROCESS_VM_WRITE | */PROCESS_QUERY_INFORMATION, FALSE, pid); //PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION
    if (_hProcess == NULL) {
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
