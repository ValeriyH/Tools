// Sandbox.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <Psapi.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <tlhelp32.h>

using namespace std;

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
    printf("\nERROR: %s. %ls. at %s:%d\n", msg, buf, file, line);
}
#define LOGERROR(msg) LogError(msg, __FILE__, __LINE__)

void ShowMemory(const MEMORY_BASIC_INFORMATION& info_in)
{
    MEMORY_BASIC_INFORMATION info = info_in;
    printf("%#10.10x %#10.10x (%6uK)\t", info.AllocationBase, info.BaseAddress, info.RegionSize / 1024);

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

    //if ((info.State == MEM_COMMIT) && (info.Type == MEM_PRIVATE))
    //    usage += info.RegionSize;

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
                wstring wstrModName = szModName;
                //you will need to change this to the name of the exe of the foreign process
                wstring wstrModContain = L".exe";
                if (wstrModName.find(wstrModContain) != wstring::npos)
                {
                    //CloseHandle(pHandle);
                    return hMods[i];
                }
            }
        }
    }
    return NULL;
}

bool SearchInProcess(HANDLE hProcess, PVOID start, SIZE_T size, DWORD dwSearch)
{
    bool ret = false;
    BYTE* dump = new BYTE[size + 1];

    memset(dump, 0x00, size + 1);
    if (ReadProcessMemory(hProcess, start, dump, size, NULL))
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
            BYTE* addr = (BYTE*)start + ((BYTE*)block_start - (BYTE*)dump);
            printf("Found %p\n", addr);
            ret = true;
            block_start++;
        }
    }

   delete[] dump;
   return ret;
}

void MemoryScan(HANDLE hProcess, DWORD value)
{
    LPVOID BaseAddress = 0; 
    HMODULE Module = GetExeModule(hProcess);
    MODULEINFO info = { 0 };
    if (GetModuleInformation(hProcess, Module, &info, sizeof(info)))
    {
        BaseAddress = info.lpBaseOfDll;
        printf("Base 0x%p\n", BaseAddress);
        printf("Image size %d\n", info.SizeOfImage);
    }
    else
    {
        LOGERROR("GetModuleInformation");
        return;
    }

    //MEMORY_BASIC_INFORMATION mbi = { 0 };
    //LPVOID Memory = BaseAddress;
    //while (VirtualQueryEx(hProcess, Memory, &mbi, sizeof(mbi)) && mbi.AllocationBase == BaseAddress)
    //{
    //    ShowMemory(mbi);
    //    SearchInProcess(hProcess, mbi.BaseAddress, mbi.RegionSize, value)
    //    Memory = (BYTE*)Memory + mbi.RegionSize;
    //}

    MEMORY_BASIC_INFORMATION mbi = { 0 };
    LPVOID Memory = 0;
    while (VirtualQueryEx(hProcess, Memory, &mbi, sizeof(mbi)))
    {
        if (mbi.State == MEM_COMMIT && (mbi.Type & MEM_PRIVATE) && ((mbi.Protect & PAGE_GUARD) == 0) && ((mbi.Protect & PAGE_NOACCESS) == 0))
        {
            if (SearchInProcess(hProcess, mbi.BaseAddress, mbi.RegionSize, value))
            {
                ShowMemory(mbi);
                printf("=============================================\n");
            }
        }
        Memory = (BYTE*)Memory + mbi.RegionSize;
    }
}

void HeapScan(HANDLE hProcess, DWORD pid, DWORD value)
{
    HEAPLIST32 hl;

    HANDLE hHeapSnap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, pid);

    hl.dwSize = sizeof(HEAPLIST32);

    if (hHeapSnap == INVALID_HANDLE_VALUE)
    {
        printf("CreateToolhelp32Snapshot failed (%d)\n", GetLastError());
        return;
    }

    if (Heap32ListFirst(hHeapSnap, &hl))
    {
        do
        {
            HEAPENTRY32 he;
            ZeroMemory(&he, sizeof(HEAPENTRY32));
            he.dwSize = sizeof(HEAPENTRY32);

            if (Heap32First(&he, pid, hl.th32HeapID))
            {
                printf("\nHeap ID: %d Flag: %X\n", hl.th32HeapID, hl.dwFlags);
                do
                {
                    printf("Addr: 0x%X Block size: %d\n", he.dwAddress, he.dwBlockSize);
                    he.dwSize = sizeof(HEAPENTRY32);
                    SearchInProcess(hProcess, (PVOID)he.dwAddress, he.dwBlockSize, value);
                } while (Heap32Next(&he));
            }
            hl.dwSize = sizeof(HEAPLIST32);
        } while (Heap32ListNext(hHeapSnap, &hl));
    }
    else printf("Cannot list first heap (%d)\n", GetLastError());

    CloseHandle(hHeapSnap);
}

void Dump(HANDLE hProcess, PVOID start, const char* path)
{
    MEMORY_BASIC_INFORMATION mbi = { 0 };
    if (VirtualQueryEx(hProcess, (LPCVOID)start, &mbi, sizeof(mbi)))
    {
        printf("BaseAddress %p Size %d\n", mbi.BaseAddress, mbi.RegionSize);
        
        char* dump = new char[mbi.RegionSize + 1];
        memset(dump, 0x00, mbi.RegionSize + 1);
    
        //Dump to file
        FILE* file = NULL;
        fopen_s(&file, path, "wb");
        if (file)
        {
            if (ReadProcessMemory(hProcess, mbi.BaseAddress, dump, mbi.RegionSize, NULL))
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
        
        delete[] dump;
        
    }

}

//NOTE: SendMessage work only with Foreground window (get keyboard imput)
//If application is runs under admin, the SendMessage under user will not work
int main()
{
    //printf("int %d\n", sizeof(int));
    //printf("long %d\n", sizeof(long));
    //printf("long long %d\n", sizeof(long long));
    //printf("void* %d\n", sizeof(void*));
    //return 0;

    HWND hWnd = FindWindowA(NULL, "Perfect World");
    if (hWnd)
    {
        DWORD pid;
        GetWindowThreadProcessId(hWnd, &pid);
        pid = 6196;
        printf("PID: %d\n", pid);

        //DWORD dwSearch;
        //printf("Enter HP: ");
        //std::cin >> dwSearch;
        PVOID start = (PVOID)0x1C8C04EC;
        const char* path = "d:\\Work\\Temp\\dump.bin";
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        //Dump to file
        #define bfz 102
        char buff[bfz];
        ZeroMemory(buff, bfz);
        FILE* file = NULL;
        fopen_s(&file, "d:\\Work\\Temp\\dump_short.bin", "wb");
        if (file)
        {
            if (ReadProcessMemory(hProcess, start, buff, bfz, NULL))
            {
                fwrite(buff, sizeof(char), bfz, file);
                DWORD* pdw = (DWORD*)buff;
                for (int i = 0; i < bfz / sizeof(DWORD); i++)
                {
                    printf("0x%x - %d\n", i * sizeof(DWORD), *(pdw + i));
                }
            }
            else
            {
                LOGERROR("ReadProcessMemory");
            }
            fclose(file);
        }

        //Dump(hProcess, start, path);
        
        //MemoryScan(hProcess, dwSearch);
        //HeapScan(hProcess, pid, dwSearch);
        //CloseHandle(hProcess);
        //return 0;

        //Sleep here. Because if we switch to foreground some keys can be handled (CTRL, ALT and etc)
        //Sleep(1000);

        HWND hCurr = GetForegroundWindow();
        SetForegroundWindow(hWnd);
        SendMessage(hWnd, WM_KEYDOWN, VK_F12, NULL);
        SendMessage(hWnd, WM_KEYUP, VK_F12, NULL);

        //SendMessage(hWnd, WM_KEYDOWN, 'B', NULL);
        //SendMessage(hWnd, WM_KEYUP, 'B', NULL);

        SendMessage(hWnd, WM_KEYDOWN, VK_F8, NULL);
        SendMessage(hWnd, WM_KEYUP, VK_F8, NULL);

        SendMessage(hWnd, WM_KEYDOWN, VK_F12, NULL);
        SendMessage(hWnd, WM_KEYUP, VK_F12, NULL);

        SetForegroundWindow(hCurr);

        //Doesn't work
        //SetForegroundWindow(hWnd);
        //BOOL res = FALSE;
        //DWORD pid = 0;
        //DWORD id = GetWindowThreadProcessId(hWnd, &pid);
        //printf("Process id %d Thread id %x\n", pid, id);
        //res = PostThreadMessage(id, WM_KEYDOWN, 'B', NULL);
        //printf("Result %d\n", res);
        //res = PostThreadMessage(id, WM_KEYUP, 'B', NULL);
        //printf("Result %d\n", res);
        
    }
    else
    {
        puts("Window not found");
    }
    return 0;
}

