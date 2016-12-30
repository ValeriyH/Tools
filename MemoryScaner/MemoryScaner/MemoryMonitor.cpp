#include "stdafx.h"
#include "MemoryMonitor.h"
#include <algorithm>

//TODO Remove this global function
extern wchar_t* GetLastErrorAsString(DWORD error = GetLastError());

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

int MemoryMonitor::InitialScan(int pid, DWORD dwSearch)
{
    if (!_OpenProcess(pid))
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

                for (; found < block_end; found = std::search(++found, block_end, token_start, token_end))
                {
                    BYTE* addr = (BYTE*)mbi.BaseAddress + (found - block_start);
                    //printf("Found at 0x%p\n", (LPVOID)addr);
                    _addrs.push_back(addr);
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
            if (!equal && data == value)
            {
                persist.push_back(addr);
            }
            if (equal && data != value)
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
