// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>

INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) 
{
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer); //TODO It can be local directory for application, not injector
    std::string path = buffer;
    path += "\\temp.txt";

    path = "D:\\temp.txt";

    /* open file */
    FILE *file;
    fopen_s(&file, path.c_str(), "a+");
    switch (Reason) {
    case DLL_PROCESS_ATTACH:
        fprintf(file, "DLL attach function called.\n");
        break;
    case DLL_PROCESS_DETACH:
        fprintf(file, "DLL detach function called.\n");
        break;
    case DLL_THREAD_ATTACH:
        fprintf(file, "DLL thread attach function called.\n");
        break;
    case DLL_THREAD_DETACH:
        fprintf(file, "DLL thread detach function called.\n");
        break;
    }
    fprintf(file, "Buffer: %s", buffer);
    /* close file */
    fclose(file);
    return TRUE;
}
