
ETW is very flexibile and you can open multiple sessions on multiple channels to write different kinds of events to different
locations.  For example, you may write application level events such as transactions processed on one channel, lower-level
resource usage such as client connect & disconnects in another, and memory allocations/frees in yet another.  

ETW is a high-speed logging facility that can be turned on and off dyanamically.  Therefore, it is very
useful for diagnosing runtime behavior of your application without invasive debugging.  It can be used to:

  Record metrics on how users interact with your application (usability data)
  Record resource consumption and release
  Record performance statistics
  Create a history of operations to verify correct application behavior
  Identify incorrect application behavior

Because Windows has the tools built in, there is nothing to install to enable it.  This makes it suitable
for server applications or any application which must stay running 24x7.  You perform offline analysis
on the data so it doesn't slow down your application.

Glossary:

   Provider:    the app which writes events
   Consumer:    the app which reads events
   Session:     the connection+buffering provided by the ETW system so a provider can send events to a consumer
   Controller:  the app which starts and stops sessions
   Channel:     a group of events for a target audience:  admin, operational, analytic, debug
   Manifest:    an XML document which describes the provider and its events to the ETW system
   Event:       a message which contains data to be logged

Events are identified by provider ID, provider-specific event ID, and event version.  They can be enabled in a session by channel, level, and keyword.  
They can also contain task and opcode data to further make filtering easier.

Channel defines target audiences for events:
   Admin        -  events which require administrator attention and action
   Operational  -  events which are for monitoring tools and support staff
   Analytic     -  events for expert-level tools and support staff
   Debug        -  events for the developers of the application

Level defines severity or verbosity of event; it makes filtering events easier.  Enabling events at level a higher level (e.g. 4) 
enables events at lower levels (3, 2, 1).  So, make 1 the most selective and the highest # the most verbose/otpional.

Keywords define subcomponents in a provider; it makes filtering events easier to narrow-down where events are coming from.

Tasks define application level operations or logical component such as process customer order
Opcodes define what the operation was such as allocate/deallocate, read/write/close, connect/disconnect, start/finish


How to determine what to log:

1. Identify the operations the application should log and the intended audience of each.  These may exist at multiple 
   levels of abstraction (e.g. high-level application-domain actions, low-level actions like allocating a buffer).
   This enables you to set the event ID, level, and variable event data.
2. Define a channel for each audience.
3. Determine which subcomponents for each component will log the events and assign them unique keywords.  Break these
   down into individual operations and assign them unique opcodes.
4. Create a template to define the variable data for each event.
5. Create the events in the manifest with ecmangen and add the EventWrite/EventWriteString calls to the places in the
   code where the events should be logged.


TODO: Check all steps below:

The key things you must create for your app to use ETW are:

1. An XML event manifest (.man file) which describes the events your application writes.  
   Your application can have multiple manifests and register itself as multiple providers.  
   (Think of each executable + manifest as a single provider.)
2. Your application (.exes and .dlls) which calls the ETW APIs and embeds a copy of the 
   manifest.  (IS THAT RIGHT???)

The big picture:

When you install your app, you register the .man file with wevtutil.exe on the same machine where the app runs.
Then, you start a session which connects your app to the ETW system so your events go somwhere.
The ETW system records you events as binary values; this makes it fast and small.
After you stop the session, you get a .etl file which contains the binary data which was logged.
You run a processor which uses the .man file to turn this binary data back into their text strings for analysis.


General API sequence:
=====================

   EventRegister
   ...
   EventWrite         -- for events in the manifest
   EventWriteString   -- for strings not in the manifest
   ...
   EventUnregister



Tools you'll use:
=================
   Tool Name         Where to get it               What it does
   =============     ===================           =========================================================
   wevtutil.exe      Comes with Windows            installs, uninstalls, and manages providers (.man files)
   logman.exe        Comes with Windows            Starts & stops tracing sessions
   tracerpt.exe      Comes with Windows            converts a raw .etl file to text by using the .man file
    
   ecmangne.exe      Windows SDK                   Helps you author a .man file
   mc.exe            Windows SDK, Visual Studo     Compiles the .man file into a .rc/.res file which is linked into your application
   xperf.exe         Windows SDK, download*        Helps you analyze an .etl file.  (*part of Windows Performance Toolkit)

Build Steps:
============
1. Run ecmangen.exe from the Windows SDK's \mssdk\bin directory to create an XML-based manifest which is named with a .man

2. Add the following pre-build command to your Visual Studio project's build steps:

   mc -um $(ProjectName).man -h $(ProjectDir)  -z $(ProjectName)

   This produces the .rc file with the manifest data compiled as .BIN.  It also creates a header
   file which contains wrapper functions that build the event descriptors used in calls to EventWrite, and
   puts strings in the event data into a message table for localization.

3. #include the file named $(ProjectName).h in your .CPP file.  This contains the definitions from the .man

4. Add logging code, such as EventWrite() and EventWriteString() calls to your code.

5. Build.  This creates the following files:

   project.man
   project.exe

Run Steps:
==========
1. Install the .man file as a provider on the machine where the app will execute.
   You may want to do this when the application is installed:

   wevtutil im project.man
   NOTE:  must be an admin to do this!!!

2. Start the session for the application

   logman start -ets project -p "Provider Name From Manifest" 0 0 -o projectlog.etl
   NOTE:  must be an admin to do this!!!

3. Run the application:

   project.exe

4. Stop the logging session:

   logman stop -ets project  
   NOTE:  must be an admin to do this!!!

5. project.etl will contain the trace in binary format.  Convert it to text on the 
   machine where the .man file is installed.  i.e. the machine where the app ran
   or a separate local machine which also has the .man file installed:

   tracerpt -y project.etl            -- to generate an XML file report
   tracerpt -y -of CSV project.etl    -- to generate a CSV file report
   OR XPERF ????

   This produces an XML or CSV file which contains all the trace data for the application.

6. Remove the .man file as a provider when it's no longer needed, such as during
   application uninstall.

   wevtutil um project.man
   NOTE:  must be an admin to do this!!!

Resources:
http://blogs.microsoft.co.il/blogs/sasha/archive/2008/03/15/xperf-windows-performance-toolkit.aspx
http://blogs.msdn.com/pigscanfly/archive/2009/08/06/stack-walking-in-xperf.aspx
http://social.msdn.microsoft.com/Forums/en-US/etw/thread/a1aa1350-41a0-4490-9ae3-9b4520aeb9d4  (FAQ)
http://windowsclient.net/wpf/white-papers/event-tracing-wpf.aspx