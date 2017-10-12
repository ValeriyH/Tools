/*-----------------------------------------------------------------------------
Example application which shows how to the manifest-based Event Tracing for 
Windows (ETW) API functions.

This application registers itself as an ETW provider named "EasyEtw" and then
calls two functions, AllocateBuffer() and FreeBuffer() that write events, inside
a loop until the user hits a key.

The message compiler (mc.exe) produces EtwExample.h, which contains constants
and wrapper functions to simplify how the application calls the ETW APIs. The
header is not shown here because it is auto-generated and contains more code
than is needed by this sample.

To run this example and enable the ETW system to capture events, follow these
steps from an administrator console window:

  1. Put EasyEtw.man in the same directory as EasyEtw.exe and register 
     EasyEtw.man as a provider:  

     wevtutil im EasyEtw.man

  2. Start the trace logging session:

     logman start -ets EtwEx -p "EasyEtw" 0 0 -o tracelog.etl

  3. Run the application:

     EasyEtw.exe

  4. Stop the logging session:

     logman stop -ets EtwEx  
 

  5. Tracelog.etl will contain the trace in binary format.  Convert it to
     a human readable format such as CSV or XML:

     tracerpt -y project.etl            -- to generate an XML file report
     tracerpt -y -of CSV project.etl    -- to generate a CSV file report

  6. Remove the .man file as a provider when it's no longer needed, such as during
     application uninstall.

     wevtutil um project.man

Example code applies to:
   Windows 7, Windows Vista
   Windows Server 2008 R2, Windows Server 2008
-----------------------------------------------------------------------------*/
#include <windows.h>
#include <evntprov.h>
#include <stdio.h>
#include <conio.h>
#include "EtwExample.h"

void * AllocateBuffer (SIZE_T byteLen);
void FreeBuffer (void * p);

int wmain (int argc, wchar_t *argv[])
{
   ULONG result;
   BYTE * pBuffer = NULL;

   // Register this program with the ETW system as a provider.
   result = EventRegisterTest_Etw_Example();  // MC.EXE generated; calls EventRegister().

   EventWriteString(Test_Etw_ExampleHandle, 0x4, 0x0, L"Application started");
   wprintf (L"Press a key to exit\n");
   while (!_kbhit())
   {
      wprintf (L"Inside Loop\n");
      
      pBuffer = (BYTE *)AllocateBuffer (8);

      Sleep(300);

      FreeBuffer (pBuffer);
   }
   EventWriteString(Test_Etw_ExampleHandle, 0x4, 0x0, L"Application finished");
   result = EventUnregisterTest_Etw_Example();  // MC.EXE generated; calls EventUnregister()

   return (0);
}

/*-----------------------------------------------------------------------------
AllocateBuffer (byteLen)

Logs an event named "Allocate" to the EtwExample/Debug channel.
-----------------------------------------------------------------------------*/
void * AllocateBuffer (SIZE_T byteLen)
{
   void * p = NULL;

   p = LocalAlloc (LPTR, byteLen);

   EventWriteBUFFER_ALLOCATED_EVENT(p, byteLen);  // MC.EXE generated; calls EventWrite()
   return (p);
}

/*-----------------------------------------------------------------------------
FreeBuffer (p)

Logs an event named "Free" to the EtwExample/Debug channel.
-----------------------------------------------------------------------------*/
void FreeBuffer (void * p)
{
   LocalFree (p);
   EventWriteBUFFER_FREED_EVENT(p);   // MC.EXE generated; calls EventWrite()
}
