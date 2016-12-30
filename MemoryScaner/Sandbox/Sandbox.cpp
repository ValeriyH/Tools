// Sandbox.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>



int main()
{
    //HWND hWnd = FindWindowA(NULL, "Calculator");
    HWND hWnd = FindWindowA(NULL, "Untitled - Notepad");
    if (hWnd)
    {
        PostMessage(hWnd, WM_KEYDOWN, VK_ESCAPE, NULL);
        PostMessage(hWnd, WM_KEYUP, VK_ESCAPE, NULL);

        PostMessage(hWnd, WM_KEYDOWN, VK_F5, NULL);
        PostMessage(hWnd, WM_KEYUP, VK_F5, NULL);
    }
    else
    {
        puts("Window not found");
    }
    return 0;
}

