#pragma once
#include <list>
//TODO
//#1 Search any type, not only DWORD
//#2 Add correct error handling
class MemoryMonitor
{
public:
    MemoryMonitor(int pid);
    virtual ~MemoryMonitor();

    //Returns number of found entries or -1 in case of error
    int FullScan(DWORD* dwSearch, int count);

    //value - search value
    //start - start block to search
    //size - block size. 0 - default. Will search to the end of allocation block
    int BlockScan(DWORD value, PVOID start, size_t size = 0);

    //Update found addresses base on new value
    //value - new value
    //equal - should the found address value equal to value.
    //  it is useful if value is changing all time (after the 1st search)
    int MemoryCorrection(DWORD value, bool equal);
    //TODO change for something like GetResults
    void ShowList();
    void ShowModules();

private:
    bool _OpenProcess();
    void _CloseProcess();
    int _pid;
    HANDLE _hProcess;
    struct MemInfo
    {
        BYTE* addr;
        BYTE* base;
    };

    std::list<MemInfo> _addrs;
};

