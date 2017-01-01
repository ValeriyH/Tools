#pragma once
#include <list>
//TODO
//#1 Search any type, not only DWORD
//#2 Add correct error handling
class MemoryMonitor
{
public:
    MemoryMonitor();
    virtual ~MemoryMonitor();

    //Returns number of found entries or -1 in case of error
    int InitialScan(int pid, DWORD value);
    int MemoryCorrection(DWORD value, bool equal);
    //TODO change for something like GetResults
    void ShowList();
    void ShowModules();

private:
    bool _OpenProcess(int pid);
    void _CloseProcess();
    HANDLE _hProcess;
    std::list<BYTE*> _addrs;
};

