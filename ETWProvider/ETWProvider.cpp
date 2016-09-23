#include "stdafx.h"
#include <evntrace.h>
#include <Windows.h>
#include <wmistr.h>
#include "ETWProvider.h"

ETWProvider::ETWProvider()
{
    _hRegistration = NULL;
    _hSession = NULL;
}


ETWProvider::~ETWProvider()
{
    Unregister();
}


ULONG ETWProvider::Register(const GUID& provider)
{
    //TODO add event class configuration
    TRACE_GUID_REGISTRATION EventClassGuids = 
    {
        &GUID_NULL,
        NULL
    };
    return ::RegisterTraceGuids(ControlCallback, this, &provider,
        sizeof(EventClassGuids) / sizeof(TRACE_GUID_REGISTRATION),
        &EventClassGuids, 
        NULL, NULL, &_hRegistration);
}

ULONG ETWProvider::Log(const GUID& provider, UCHAR type, UCHAR level, char* message)
{
    if (NULL == _hSession || _level < level)
    {
        return ERROR_SUCCESS; 
    }

    EtwMofEvent<1> event(event_class, type, level);

    event.fields[0].DataPtr = reinterpret_cast<ULONG64>(message);
    event.fields[0].Length = message ?
        static_cast<ULONG>(sizeof(message[0]) * (1 + wcslen(message))) : 0;

    return ::TraceEvent(_hSession, &event.header);
}

ULONG ETWProvider::Unregister()
{
    return true;
}


// The callback function that receives enable/disable notifications
// from one or more ETW sessions. Because more than one session
// can enable the provider, this example ignores requests from other 
// sessions if it is already enabled.

ULONG WINAPI
ETWProvider::ControlCallback(
    WMIDPREQUESTCODE RequestCode,
    PVOID Context,
    ULONG* Reserved,
    PVOID Header
    )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Reserved);

    ULONG status = ERROR_SUCCESS;
    TRACEHANDLE TempSessionHandle = 0;

    switch (RequestCode)
    {
    case WMI_ENABLE_EVENTS:  // Enable the provider.
    {
        SetLastError(0);

        // If the provider is already enabled to a provider, ignore 
        // the request. Get the session handle of the enabling session.
        // You need the session handle to call the TraceEvent function.
        // The session could be enabling the provider or it could be
        // updating the level and enable flags.

        TempSessionHandle = GetTraceLoggerHandle(Header);
        if (INVALID_HANDLE_VALUE == (HANDLE)TempSessionHandle)
        {
            wprintf(L"GetTraceLoggerHandle failed. Error code is %lu.\n", status = GetLastError());
            break;
        }

        if (0 == g_SessionHandle)
        {
            g_SessionHandle = TempSessionHandle;
        }
        else if (g_SessionHandle != TempSessionHandle)
        {
            break;
        }

        // Get the severity level of the events that the
        // session wants you to log.

        g_EnableLevel = GetTraceEnableLevel(g_SessionHandle);
        if (0 == g_EnableLevel)
        {
            // If zero, determine whether the session passed zero
            // or an error occurred.

            if (ERROR_SUCCESS == (status = GetLastError()))
            {
                // Decide what a zero enable level means to your provider.
                // For this example, it means log all events.
                ;
            }
            else
            {
                wprintf(L"GetTraceEnableLevel failed with, %lu.\n", status);
                break;
            }
        }

        // Get the enable flags that indicate the events that the
        // session wants you to log. The provider determines the
        // flags values. How it articulates the flag values and 
        // meanings to perspective sessions is up to it.

        g_EnableFlags = GetTraceEnableFlags(g_SessionHandle);
        if (0 == g_EnableFlags)
        {
            // If zero, determine whether the session passed zero
            // or an error occurred.

            if (ERROR_SUCCESS == (status = GetLastError()))
            {
                // Decide what a zero enable flags value means to your provider.
                ;
            }
            else
            {
                wprintf(L"GetTraceEnableFlags failed with, %lu.\n", status);
                break;
            }
        }

        g_TraceOn = TRUE;
        break;
    }

    case WMI_DISABLE_EVENTS:  // Disable the provider.
    {
        // Disable the provider only if the request is coming from the
        // session that enabled the provider.

        TempSessionHandle = GetTraceLoggerHandle(Header);
        if (INVALID_HANDLE_VALUE == (HANDLE)TempSessionHandle)
        {
            wprintf(L"GetTraceLoggerHandle failed. Error code is %lu.\n", status = GetLastError());
            break;
        }

        if (g_SessionHandle == TempSessionHandle)
        {
            g_TraceOn = FALSE;
            g_SessionHandle = 0;
        }
        break;
    }

    default:
    {
        status = ERROR_INVALID_PARAMETER;
        break;
    }
    }

    return status;
}