//**********************************************************************`
//* This is an include file generated by Message Compiler.             *`
//*                                                                    *`
//* Copyright (c) Microsoft Corporation. All Rights Reserved.          *`
//**********************************************************************`
#pragma once
#include <wmistr.h>
#include <evntrace.h>
#include "evntprov.h"
//
//  Initial Defs
//
#if !defined(ETW_INLINE)
#define ETW_INLINE DECLSPEC_NOINLINE __inline
#endif

#if defined(__cplusplus)
extern "C" {
#endif

//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION
#if  !defined(McGenDebug)
#define McGenDebug(a,b)
#endif 


#if !defined(MCGEN_TRACE_CONTEXT_DEF)
#define MCGEN_TRACE_CONTEXT_DEF
typedef struct _MCGEN_TRACE_CONTEXT
{
    TRACEHANDLE            RegistrationHandle;
    TRACEHANDLE            Logger;
    ULONGLONG              MatchAnyKeyword;
    ULONGLONG              MatchAllKeyword;
    ULONG                  Flags;
    ULONG                  IsEnabled;
    UCHAR                  Level; 
    UCHAR                  Reserve;
    USHORT                 EnableBitsCount;
    PULONG                 EnableBitMask;
    const ULONGLONG*       EnableKeyWords;
    const UCHAR*           EnableLevel;
} MCGEN_TRACE_CONTEXT, *PMCGEN_TRACE_CONTEXT;
#endif

#if !defined(MCGEN_LEVEL_KEYWORD_ENABLED_DEF)
#define MCGEN_LEVEL_KEYWORD_ENABLED_DEF
FORCEINLINE
BOOLEAN
McGenLevelKeywordEnabled(
    _In_ PMCGEN_TRACE_CONTEXT EnableInfo,
    _In_ UCHAR Level,
    _In_ ULONGLONG Keyword
    )
{
    //
    // Check if the event Level is lower than the level at which
    // the channel is enabled.
    // If the event Level is 0 or the channel is enabled at level 0,
    // all levels are enabled.
    //

    if ((Level <= EnableInfo->Level) || // This also covers the case of Level == 0.
        (EnableInfo->Level == 0)) {

        //
        // Check if Keyword is enabled
        //

        if ((Keyword == (ULONGLONG)0) ||
            ((Keyword & EnableInfo->MatchAnyKeyword) &&
             ((Keyword & EnableInfo->MatchAllKeyword) == EnableInfo->MatchAllKeyword))) {
            return TRUE;
        }
    }

    return FALSE;

}
#endif

#if !defined(MCGEN_EVENT_ENABLED_DEF)
#define MCGEN_EVENT_ENABLED_DEF
FORCEINLINE
BOOLEAN
McGenEventEnabled(
    _In_ PMCGEN_TRACE_CONTEXT EnableInfo,
    _In_ PCEVENT_DESCRIPTOR EventDescriptor
    )
{

    return McGenLevelKeywordEnabled(EnableInfo, EventDescriptor->Level, EventDescriptor->Keyword);

}
#endif


//
// EnableCheckMacro
//
#ifndef MCGEN_ENABLE_CHECK
#define MCGEN_ENABLE_CHECK(Context, Descriptor) (Context.IsEnabled &&  McGenEventEnabled(&Context, &Descriptor))
#endif

#if !defined(MCGEN_CONTROL_CALLBACK)
#define MCGEN_CONTROL_CALLBACK

DECLSPEC_NOINLINE __inline
VOID
__stdcall
McGenControlCallbackV2(
    _In_ LPCGUID SourceId,
    _In_ ULONG ControlCode,
    _In_ UCHAR Level,
    _In_ ULONGLONG MatchAnyKeyword,
    _In_ ULONGLONG MatchAllKeyword,
    _In_opt_ PEVENT_FILTER_DESCRIPTOR FilterData,
    _Inout_opt_ PVOID CallbackContext
    )
/*++

Routine Description:

    This is the notification callback for Vista.

Arguments:

    SourceId - The GUID that identifies the session that enabled the provider. 

    ControlCode - The parameter indicates whether the provider 
                  is being enabled or disabled.

    Level - The level at which the event is enabled.

    MatchAnyKeyword - The bitmask of keywords that the provider uses to 
                      determine the category of events that it writes.

    MatchAllKeyword - This bitmask additionally restricts the category 
                      of events that the provider writes. 

    FilterData - The provider-defined data.

    CallbackContext - The context of the callback that is defined when the provider 
                      called EtwRegister to register itself.

Remarks:

    ETW calls this function to notify provider of enable/disable

--*/
{
    PMCGEN_TRACE_CONTEXT Ctx = (PMCGEN_TRACE_CONTEXT)CallbackContext;
    ULONG Ix;
#ifndef MCGEN_PRIVATE_ENABLE_CALLBACK_V2
    UNREFERENCED_PARAMETER(SourceId);
    UNREFERENCED_PARAMETER(FilterData);
#endif

    if (Ctx == NULL) {
        return;
    }

    switch (ControlCode) {

        case EVENT_CONTROL_CODE_ENABLE_PROVIDER:
            Ctx->Level = Level;
            Ctx->MatchAnyKeyword = MatchAnyKeyword;
            Ctx->MatchAllKeyword = MatchAllKeyword;
            Ctx->IsEnabled = EVENT_CONTROL_CODE_ENABLE_PROVIDER;

            for (Ix = 0; Ix < Ctx->EnableBitsCount; Ix += 1) {
                if (McGenLevelKeywordEnabled(Ctx, Ctx->EnableLevel[Ix], Ctx->EnableKeyWords[Ix]) != FALSE) {
                    Ctx->EnableBitMask[Ix >> 5] |= (1 << (Ix % 32));
                } else {
                    Ctx->EnableBitMask[Ix >> 5] &= ~(1 << (Ix % 32));
                }
            }
            break;

        case EVENT_CONTROL_CODE_DISABLE_PROVIDER:
            Ctx->IsEnabled = EVENT_CONTROL_CODE_DISABLE_PROVIDER;
            Ctx->Level = 0;
            Ctx->MatchAnyKeyword = 0;
            Ctx->MatchAllKeyword = 0;
            if (Ctx->EnableBitsCount > 0) {
                RtlZeroMemory(Ctx->EnableBitMask, (((Ctx->EnableBitsCount - 1) / 32) + 1) * sizeof(ULONG));
            }
            break;
 
        default:
            break;
    }

#ifdef MCGEN_PRIVATE_ENABLE_CALLBACK_V2
    //
    // Call user defined callback
    //
    MCGEN_PRIVATE_ENABLE_CALLBACK_V2(
        SourceId,
        ControlCode,
        Level,
        MatchAnyKeyword,
        MatchAllKeyword,
        FilterData,
        CallbackContext
        );
#endif
   
    return;
}

#endif
#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION
//+
// Provider EtwExample Event Count 3
//+
EXTERN_C __declspec(selectany) const GUID ETW_EXAMPLE_PROVIDER = {0xfe2625c1, 0xc10d, 0x452c, {0xb8, 0x13, 0xa8, 0x70, 0x3e, 0xa9, 0xd2, 0xba}};

//
// Channel
//
#define DEBUG_CHANNEL 0x10
#define ANALYTIC_CHANNEL 0x11

//
// Opcodes
//
#define ALLOCATE_OPCODE 0xa
#define FREE_OPCODE 0xb
//
// Keyword
//
#define BUFFER_MANAGER_KEYWORD 0x800000000000

//
// Event Descriptors
//
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR BUFFER_ALLOCATED_EVENT = {0x1, 0x1, 0x10, 0x4, 0xa, 0x0, 0x8000800000000000};
#define BUFFER_ALLOCATED_EVENT_value 0x1
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR BUFFER_FREED_EVENT = {0x2, 0x1, 0x10, 0x4, 0xb, 0x0, 0x8000800000000000};
#define BUFFER_FREED_EVENT_value 0x2
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR MESSAGE = {0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
#define MESSAGE_value 0x3

//
// Note on Generate Code from Manifest Windows Vista and above
//
//Structures :  are handled as a size and pointer pairs. The macro for the event will have an extra 
//parameter for the size in bytes of the structure. Make sure that your structures have no extra padding.
//
//Strings: There are several cases that can be described in the manifest. For array of variable length 
//strings, the generated code will take the count of characters for the whole array as an input parameter. 
//
//SID No support for array of SIDs, the macro will take a pointer to the SID and use appropriate 
//GetLengthSid function to get the length.
//

//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//
// Globals 
//


//
// Event Enablement Bits
//

EXTERN_C __declspec(selectany) DECLSPEC_CACHEALIGN ULONG EtwExampleEnableBits[1];
EXTERN_C __declspec(selectany) const ULONGLONG EtwExampleKeywords[2] = {0x8000800000000000, 0x0};
EXTERN_C __declspec(selectany) const UCHAR EtwExampleLevels[2] = {4, 0};
EXTERN_C __declspec(selectany) MCGEN_TRACE_CONTEXT ETW_EXAMPLE_PROVIDER_Context = {0, 0, 0, 0, 0, 0, 0, 0, 2, EtwExampleEnableBits, EtwExampleKeywords, EtwExampleLevels};

EXTERN_C __declspec(selectany) REGHANDLE EtwExampleHandle = (REGHANDLE)0;

#if !defined(McGenEventRegisterUnregister)
#define McGenEventRegisterUnregister
DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventRegister(
    _In_ LPCGUID ProviderId,
    _In_opt_ PENABLECALLBACK EnableCallback,
    _In_opt_ PVOID CallbackContext,
    _Inout_ PREGHANDLE RegHandle
    )
/*++

Routine Description:

    This function register the provider with ETW USER mode.

Arguments:
    ProviderId - Provider Id to be register with ETW.

    EnableCallback - Callback to be used.

    CallbackContext - Context for this provider.

    RegHandle - Pointer to Registration handle.

Remarks:

    If the handle != NULL will return ERROR_SUCCESS

--*/
{
    ULONG Error;


    if (*RegHandle) {
        //
        // already registered
        //
        return ERROR_SUCCESS;
    }

    Error = EventRegister( ProviderId, EnableCallback, CallbackContext, RegHandle); 

    return Error;
}


DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventUnregister(_Inout_ PREGHANDLE RegHandle)
/*++

Routine Description:

    Unregister from ETW USER mode

Arguments:
            RegHandle this is the pointer to the provider context
Remarks:
            If Provider has not register RegHandle = NULL,
            return ERROR_SUCCESS
--*/
{
    ULONG Error;


    if(!(*RegHandle)) {
        //
        // Provider has not registerd
        //
        return ERROR_SUCCESS;
    }

    Error = EventUnregister(*RegHandle); 
    *RegHandle = (REGHANDLE)0;
    
    return Error;
}
#endif
//
// Register with ETW Vista +
//
#ifndef EventRegisterEtwExample
#define EventRegisterEtwExample() McGenEventRegister(&ETW_EXAMPLE_PROVIDER, McGenControlCallbackV2, &ETW_EXAMPLE_PROVIDER_Context, &EtwExampleHandle) 
#endif

//
// UnRegister with ETW
//
#ifndef EventUnregisterEtwExample
#define EventUnregisterEtwExample() McGenEventUnregister(&EtwExampleHandle) 
#endif

//
// Enablement check macro for BUFFER_ALLOCATED_EVENT
//

#define EventEnabledBUFFER_ALLOCATED_EVENT() ((EtwExampleEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for BUFFER_ALLOCATED_EVENT
//
#define EventWriteBUFFER_ALLOCATED_EVENT(BaseAddress, Length)\
        EventEnabledBUFFER_ALLOCATED_EVENT() ?\
        Template_px(EtwExampleHandle, &BUFFER_ALLOCATED_EVENT, BaseAddress, Length)\
        : ERROR_SUCCESS\

//
// Enablement check macro for BUFFER_FREED_EVENT
//

#define EventEnabledBUFFER_FREED_EVENT() ((EtwExampleEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for BUFFER_FREED_EVENT
//
#define EventWriteBUFFER_FREED_EVENT(BaseAddress)\
        EventEnabledBUFFER_FREED_EVENT() ?\
        Template_p(EtwExampleHandle, &BUFFER_FREED_EVENT, BaseAddress)\
        : ERROR_SUCCESS\

//
// Enablement check macro for MESSAGE
//

#define EventEnabledMESSAGE() ((EtwExampleEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for MESSAGE
//
#define EventWriteMESSAGE()\
        EventEnabledMESSAGE() ?\
        TemplateEventDescriptor(EtwExampleHandle, &MESSAGE)\
        : ERROR_SUCCESS\

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION


//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//
// Template Functions 
//
//
//Template from manifest : BufferAllocate
//
#ifndef Template_px_def
#define Template_px_def
ETW_INLINE
ULONG
Template_px(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_opt_ const void *  _Arg0,
    _In_ unsigned __int64  _Arg1
    )
{
#define ARGUMENT_COUNT_px 2

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_px];

    EventDataDescCreate(&EventData[0], &_Arg0, sizeof(PVOID)  );

    EventDataDescCreate(&EventData[1], &_Arg1, sizeof(unsigned __int64)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_px, EventData);
}
#endif

//
//Template from manifest : BufferFree
//
#ifndef Template_p_def
#define Template_p_def
ETW_INLINE
ULONG
Template_p(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_opt_ const void *  _Arg0
    )
{
#define ARGUMENT_COUNT_p 1

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_p];

    EventDataDescCreate(&EventData[0], &_Arg0, sizeof(PVOID)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_p, EventData);
}
#endif

//
//Template from manifest : (null)
//
#ifndef TemplateEventDescriptor_def
#define TemplateEventDescriptor_def


ETW_INLINE
ULONG
TemplateEventDescriptor(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor
    )
{
    return EventWrite(RegHandle, Descriptor, 0, NULL);
}
#endif

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION

#if defined(__cplusplus)
};
#endif

#define MSG_level_Informational              0x50000004L
#define MSG_EtwExample_event_1_message       0xB0010001L
#define MSG_EtwExample_event_2_message       0xB0010002L