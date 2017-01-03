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
    DWORD id = 0;
    DWORD search[MAX_PATH];
    int count = 0;
    DWORD base = 0;

    if (argc < 3)
    {
        printf("Usage: MemoryScaner <PID> <SEARCH DWORD>\n");
        printf("Enter PID: ");
        std::cin >> id;
        printf("Enter search: ");
        std::cin >> search[0];
        count = 1;
    }
    else
    {
        id = atoi(argv[1]);
        
        printf("Search [");
        char * pch = strtok(argv[2], " ");
        for (int i = 0; pch && i < MAX_PATH; i++, pch = strtok(NULL, " "))
        {
            DWORD dw = atoi(pch);
            printf(" %d", dw);
            search[i] = dw;
            count++;
        }
        printf(" ]\n");
    }

    if (argc > 3)
    {
        base = strtol(argv[3], NULL, 16);
    }

    int res = 0;
    MemoryMonitor monitor(id);
    
    DWORD start = GetTickCount();
    printf("Process %d. Base 0x%X\nSearching...\n", id, base);
    if (!base)
    {
        res = monitor.FullScan(search, count);
    }
    else
    {
        res = monitor.BlockScan(search[0], (LPVOID)base);
    }
    printf("Found %d items. During %d ticks\n", res, GetTickCount() - start);

    do
    {
        printf("Recheking 1st found values with %d\n", search[0]);
        res = monitor.MemoryCorrection(search[0], true);
        printf("Found %d items\n", res);

        puts("Searching will stop if found values will be less than 10");
        if (res < 10)
        {
            break;
        }

        printf("Input 1st new value or press Enter to rechek %d:", search[0]);
        std::string str;
        std::cin >> str;
        if (!str.empty())
        {
            search[0] = atoi(str.c_str());
        }
    } while (true);
 
    do
    {
        monitor.ShowList();
        Sleep(1000);
        puts("----");
        getchar();
    } while (true);
   
    return 0;
}

