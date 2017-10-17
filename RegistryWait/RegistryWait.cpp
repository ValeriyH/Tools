// RegistryWait.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Windows.h>

class RegistryWait
{
private:
    HKEY _hKey;
    HANDLE _hEvent;
    HANDLE _hWait;


    static VOID CALLBACK WaitOrTimerCallback(
        _In_ PVOID   lpParameter,
        _In_ BOOLEAN TimerOrWaitFired
    )
    {
        //NOTE: The callback can be false positive for following reasons:
        //This function detects a single change. After the caller receives a notification event, it should call the function again to receive the next notification.
        //1. If the thread that called RegNotifyChangeKeyValue exits, the event is signaled. (callback is from non-persistent thread pool)
        //2. If the specified key is closed, the event is signaled.
        // RegNotifyChangeKeyValue will raise event then thread will terminated
        // Starting from windows 8 this issue fixed with REG_NOTIFY_THREAD_AGNOSTIC flag
        //
        //Details: https://msdn.microsoft.com/en-us/library/windows/desktop/ms724892.aspx?f=255&MSPPError=-2147217396
        ((RegistryWait*)lpParameter)->MonitorChanges();
        _tprintf(TEXT("DATA Modified.\n"));
    }

    void MonitorChanges()
    {
        //NOTE: RegNotifyChangeKeyValue should be recalled each time when change notification received
        RegNotifyChangeKeyValue(_hKey, FALSE, REG_NOTIFY_CHANGE_LAST_SET, _hEvent, TRUE);
    }

    bool Initialize(HKEY rootKey, char* subKey)
    {
        LSTATUS lRet = ERROR_SUCCESS;
        _hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        //TODO handle errors. 
        //TODO if fail create try open existing one. (NOTE: create can be done by admin user, open by any user)
        lRet = RegCreateKeyExA(rootKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &_hKey, NULL);
        RegisterWaitForSingleObject(&_hWait, _hEvent, WaitOrTimerCallback, this, INFINITE, WT_EXECUTELONGFUNCTION);
        MonitorChanges();

        return true;
    }

public:
    RegistryWait(char*  key)
    {
        Initialize(HKEY_LOCAL_MACHINE, key);
    }

    ~RegistryWait()
    {
        //TODO Unregister all features
        if (_hKey)
        {
            RegCloseKey(_hKey);
            _hKey = NULL;
        }
    }
};


int main()
{
    RegistryWait test("SOFTWARE\\Test");

    while (true)
    {

        Sleep(100);
    }
   

    return 0;
}

