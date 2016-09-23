// ETWProvider.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <evntrace.h>
#include "ETWProvider.h"
#include "ETW.h"

// {E138B5A6-5015-4938-B7D9-A9B64857B7C2}
static const GUID ProviderGuid =
{ 0xe138b5a6, 0x5015, 0x4938,{ 0xb7, 0xd9, 0xa9, 0xb6, 0x48, 0x57, 0xb7, 0xc2 } };

// {B49D5931-AD85-4070-B1B1-3F81F1532875}
static const GUID EventCategory =
{ 0xb49d5931, 0xad85, 0x4070,{ 0xb1, 0xb1, 0x3f, 0x81, 0xf1, 0x53, 0x28, 0x75 } };

int main()
{
    ETWProvider tracer;
    tracer.Register(ProviderGuid);
    tracer.Log(EventCategory, EVENT_TRACE_TYPE_INFO, TRACE_LEVEL_FATAL, "test");
    tracer.Unregister();
    return 0;
}

