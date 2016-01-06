// Unique.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <set>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        puts("This tool check what all lines in the file are unique. All non-unique lines printed.");
        puts("Error level\n\tset to 0 if all lines are unique.\n\tset to 1 if not all arguments is set or fail to open file.\n\tset to 2 if some lines are not unique.");
        printf("Usage %s <filename>\n", argv[0]);
        return 1;
    }
    else
    {
        printf("Reading file %s\n", argv[1]);
    }

    int ret = 0;
    FILE * pFile = NULL;
    char mystring[100];

    fopen_s(&pFile, argv[1], "r");
    if (pFile == NULL)
    {
        perror("Error opening file");
        return 1;
    }
    else
    {
        int size = 0;
        std::set<std::string> strings;

        while (fgets(mystring, 100, pFile) != NULL)
        {
            strings.insert(std::string(mystring));
            if (size == strings.size())
            {
                printf("Not unique: %s", mystring);
                ret = 2;
            }
            size = strings.size();
        }
        fclose(pFile);
    }

    return ret;
}

