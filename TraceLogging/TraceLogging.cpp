// TraceLogging.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SimpleTraceLoggingExample.h"

// Define the GUID to use in TraceLoggingProviderRegister 
// {3970F9cf-2c0c-4f11-b1cc-e3a1e9958833}
TRACELOGGING_DEFINE_PROVIDER(
    g_hMyComponentProvider,
    "SimpleTraceLoggingProvider",
    (0x3970f9cf, 0x2c0c, 0x4f11, 0xb1, 0xcc, 0xe3, 0xa1, 0xe9, 0x95, 0x88, 0x33));

int main()
{
    char sampleValue[] = "Sample value";

    // Register the provider
    TraceLoggingRegister(g_hMyComponentProvider);
    // Log an event
    TraceLoggingWrite(g_hMyComponentProvider, // handle to my provider
        "HelloWorldTestEvent",              // Event Name that should uniquely identify your event.
        TraceLoggingValue(sampleValue, "TestMessage")); // Field for your event in the form of (value, field name).
                                                        // Stop TraceLogging and unregister the provider
    TraceLoggingUnregister(g_hMyComponentProvider);

    return 0;
}

