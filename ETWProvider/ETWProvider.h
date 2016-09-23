#pragma once

#include <windows.h>
#include <wmistr.h>
#include <evntrace.h>
#include <stddef.h>
#include <stdint.h>

class ETWProvider
{
public:
    ETWProvider();
    ~ETWProvider();

    //Returns ::RegisterTraceGuids error code 
    ULONG Register(const GUID& provider);
    ULONG Log(const GUID& provider, UCHAR type, UCHAR level, char* message);
    ULONG Unregister();

private:

    static ULONG WINAPI ControlCallback(WMIDPREQUESTCODE RequestCode, PVOID Context, ULONG* Reserved, PVOID Header);

    TRACEHANDLE _hRegistration;
    TRACEHANDLE _hSession;
    UCHAR _level;
};

