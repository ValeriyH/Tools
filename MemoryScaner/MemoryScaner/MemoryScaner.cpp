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
#include "MemoryMonitor.h"

wchar_t* GetLastErrorAsString(DWORD error = GetLastError())
{
    static wchar_t buf[256];
    buf[0] = 0;
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
        MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), buf, sizeof(buf), NULL);
    return buf;
}

int main(int argc, char *argv[])
{
    //_setmode(_fileno(stdout), _O_U16TEXT);
    if (argc < 3)
    {
        printf("Usage: MemoryScaner <PID> <SEARCH DWORD>\n");
        return 1;
    }

    int res = 0;
    DWORD id = atoi(argv[1]);
    DWORD dwSearch = atoi(argv[2]);

    MemoryMonitor monitor;
    
    printf("Searching %d process memory %d...\n", dwSearch, id);
    res = monitor.InitialScan(id, dwSearch);
    printf("Found %d items\n", res);

    do
    {
        printf("Recheking found values with %d\n", dwSearch);
        res = monitor.MemoryCorrection(dwSearch, true);
        printf("Found %d items\n", res);

        puts("Searching will stop if found values will be less than 10");
        if (res < 10)
        {
            break;
        }

        printf("Input new value or press Enter to rechek %d:", dwSearch);
        std::string str;
        std::cin >> str;
        if (!str.empty())
        {
            dwSearch = atoi(str.c_str());
        }
    } while (true);
 
    do
    {
        monitor.ShowList();
        Sleep(1000);
        puts("");
    } while (true);
   
    return 0;
}

