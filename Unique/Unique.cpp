// Unique.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <map>

#define MAX_LINE_SIZE 4096

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        puts("This tool check what all lines in the file are unique. All non-unique lines printed.");
        puts("Error level\n\tset to 0 if all lines are unique.\n\tset to 1 if not all arguments is set or fail to open file.\n\tset to 2 if some lines are not unique.");
        printf("Note: line size is limited to %d chars\n\n", MAX_LINE_SIZE);
        printf("Usage %s <filename>\n", argv[0]);
        return 1;
    }
    else
    {
        printf("Reading file %s\n", argv[1]);
    }

    int ret = 0;
    FILE * pFile = NULL;
    char mystring[MAX_LINE_SIZE];

    fopen_s(&pFile, argv[1], "r");
    if (pFile == NULL)
    {
        perror("Error opening file");
        return 1;
    }
    else
    {
        std::map<std::string, int> matched;
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
    }

    return ret;
}

