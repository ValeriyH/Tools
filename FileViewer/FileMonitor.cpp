#include "stdafx.h"
#include "FileMonitor.h"
#include <vector>

FileMonitor::FileMonitor()
{
}


FileMonitor::~FileMonitor()
{
}


void FileMonitor::StartMonitor(std::wstring filename, HWND hEdit)
{
    StopMonitor();

    _sFilename = filename;
    _hEdit = hEdit;

    //TODO Use file mapping with own edit window
    //TODO Monitor file delete operation and close file
    _hFile = CreateFile(_sFilename.c_str(),
        GENERIC_READ,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);

    StartThread();
}

void FileMonitor::StopMonitor()
{
    StopThread();
    CloseHandle(_hFile);
    _sFilename = L"";
    _hEdit = NULL;
    _hFile = NULL;
}

void FileMonitor::WorkerThread()
{
    //TODO Monitor file changes
    //TODO Read file async

    LARGE_INTEGER file_size_prev = { 0 };
    std::vector<char> buff;
    while (1)
    {
        LARGE_INTEGER file_size = { 0 };
        Edit_LimitText(_hEdit, -1);
        if (GetFileSizeEx(_hFile, &file_size) && file_size.QuadPart != file_size_prev.QuadPart)
        {
            file_size_prev = file_size;
            DWORD dwRead = 0;
            //char* buff = new char[file_size.QuadPart + 1];
            buff.resize(file_size.QuadPart + 1);
            //ZeroMemory(buff, file_size.QuadPart + 1);
            if (buff.capacity())
            {
                SetFilePointer(_hFile, 0, NULL, FILE_BEGIN);
                if (ReadFile(_hFile, &buff[0], file_size_prev.QuadPart, &dwRead, NULL))
                {
                    buff[file_size.QuadPart] = 0;
                    SetWindowTextA(_hEdit, &buff[0]);
                    //TODO if(autoscroll)
                    int lines = Edit_GetLineCount(_hEdit);
                    SendMessage(_hEdit, EM_LINESCROLL, 0, lines);

                    //Apend text to edit box. Clean/Restore selection/cursor
                    //DWORD l, r;
                    //SendMessage(_hEdit, EM_GETSEL, (WPARAM)&l, (LPARAM)&r);
                    //SendMessage(_hEdit, EM_SETSEL, 0, -1);
                    //SendMessageA(_hEdit, EM_REPLACESEL, 0, (LPARAM)buff);
                    //Edit_ScrollCaret(_hEdit);
                    //SendMessage(_hEdit, EM_SETSEL, l, r);
                }
            }
            else
            {
                SetWindowText(_hEdit, L"ERROR! OUT OF MEMORY");
            }
        }
        Sleep(1000);
    }

}