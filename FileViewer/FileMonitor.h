#pragma once

template <class T>
class Thread
{
    DWORD dwThreadId = 0;
    HANDLE hThread = NULL;
public:
    Thread()
    {
    }

    virtual ~Thread() {};

    void StartThread()
    {
        //obj = dynamic_cast<T*>(this);
        hThread = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            Thread::ThreadFunct,                 // thread function name
            this,                  // argument to thread function 
            0,                   // use default creation flags 
            &dwThreadId);       // returns the thread identifier 
    }

    void StopThread()
    {
        //TODO Terminate correctly. Signal and wait. After terminate
        TerminateThread(hThread, 0);
        hThread = NULL;
        dwThreadId = 0;
    }

    static DWORD WINAPI ThreadFunct(LPVOID lpParam)
    {
        T* obj = static_cast<T*>(lpParam);
        if (obj)
        {
            obj->WorkerThread();
        }
        return 0;
    }

};


class FileMonitor
    :public Thread<FileMonitor>
{
public:
    FileMonitor();
    virtual ~FileMonitor();

    void StartMonitor(std::wstring filename, HWND hEdit);
    void StopMonitor();

    void WorkerThread();

private:
    std::wstring _sFilename;
    std::wstring _sData;
    HWND _hEdit;
    HANDLE _hFile;
};

