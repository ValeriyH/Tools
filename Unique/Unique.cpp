// Unique.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <map>
#include <Windows.h>

#define MAX_LINE_SIZE 4096

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        puts("This tool check what all lines in the file are unique. All non-unique lines printed.");
        puts("Error level\n\tset to 0 if all lines are unique.\n\tset to 1 if not all arguments is set or fail to open file.\n\tset to 2 if some lines are not unique.");
        printf("Note: line size is limited to %d chars\n\n", MAX_LINE_SIZE);
        printf("Note: folder should end with \\ character");
        printf("Usage %s <folder> <filename_pattern>\n", argv[0]);
        return 1;
    }

    std::string dir = argv[1];
    std::string file_pattern = argv[2];
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile((dir + file_pattern).c_str(), &data)) == INVALID_HANDLE_VALUE)
    {
        perror("Can't open folder");
        return 1;
    }

    int ret = 0;
    std::map<std::string, int> matched;
    do 
    {
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            continue;
        }

        printf("Reading file %s\n", data.cFileName);
        FILE* pFile = NULL;
        char mystring[MAX_LINE_SIZE];

        fopen_s(&pFile, (dir + data.cFileName).c_str(), "r");
        if (pFile == NULL)
        {
            perror("Error opening file:");
            perror(data.cFileName);
            ret = ret == 0 ? 1 : ret;
            continue;
        }
        else
        {
             while (fgets(mystring, MAX_LINE_SIZE, pFile) != NULL)
            {
                int len = strlen(mystring);
                if (mystring[len - 2] == '\r')
                {
                    mystring[len - 2] = 0;
                }
                else if (mystring[len - 1] == '\n')
                {
                    mystring[len - 1] = 0;
                }

                matched[mystring]++;
            }
            fclose(pFile);
        }
    } 
    while (FindNextFile(hFind, &data) != 0);
    FindClose(hFind);

    for each (auto line in matched)
    {
        if (line.second > 1)
        {
            printf("Line '%s' occurs %d times\n", line.first.c_str(), line.second);
            ret = 2;
        }
    }

    if (ret == 0)
    {
        puts("All lines are uniuqe.");
    }


    return ret;
}

