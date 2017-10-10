Project original https://stackoverflow.com/questions/8559222/write-an-event-to-the-event-viewer

Modifications:
1] Added more includes
2] Added "mc.exe -A -b -c -h . -r resources Event_log.mc" to pre-build events with full path $(ProjectDir)
3] Added Event_log.mc to project "Resource file". i.e link to project
4] Change application path in the code: BYTE exe_path[] = "C:\\path\\to\\your\\application.exe";

Custom events:
http://www.jasonsamuel.com/2010/01/08/creating-a-custom-event-log-under-event-viewer-to-log-server-events/
Note: For custom events RegisterEventSource receive source as input source. No information about custom level in the code.

Create event from console
eventcreate /l CustomLog /t Information /so Application1 /id 1 /d "Test message"

=======================================================================================================================================
Here is a quick example of how to use these and to display messages correctly in the event log (error handling mostly ignored for brevity).

Create a resource containg message information from the following Event_log.mc file:
=======================================================================================================================================

;#ifndef _EXAMPLE_EVENT_LOG_MESSAGE_FILE_H_
;#define _EXAMPLE_EVENT_LOG_MESSAGE_FILE_H_

MessageIdTypeDef=DWORD


SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
               )

LanguageNames=(EnglishUS=0x401:MSG00401
               Dutch=0x113:MSG00113
               Neutral=0x0000:MSG00000
               )

MessageId=0x0   SymbolicName=MSG_INFO_1
Severity=Informational
Facility=Application
Language=Neutral
%1
.

MessageId=0x1   SymbolicName=MSG_WARNING_1
Severity=Warning
Facility=Application
Language=Neutral
%1
.

MessageId=0x2   SymbolicName=MSG_ERROR_1
Severity=Error
Facility=Application
Language=Neutral
%1
.

MessageId=0x3   SymbolicName=MSG_SUCCESS_1
Severity=Success
Facility=Application
Language=Neutral
%1
.


;#endif


=======================================================================================================================================
To build the .mc file and .res resource file I executed the following:

mc.exe -A -b -c -h . -r resources Event_log.mc
rc.exe -foresources/Event_log.res resources/Event_log.rc
This will create a header file called Event_log.h in the current directory and a resources directory containing a file named Event_log.res which you must link in to your application binary.

Example main.cpp:
=======================================================================================================================================

#include <windows.h>
#include "Event_log.h"

void install_event_log_source(const std::string& a_name)
{
    const std::string key_path("SYSTEM\\CurrentControlSet\\Services\\"
                               "EventLog\\Application\\" + a_name);

    HKEY key;

    DWORD last_error = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                      key_path.c_str(),
                                      0,
                                      0,
                                      REG_OPTION_NON_VOLATILE,
                                      KEY_SET_VALUE,
                                      0,
                                      &key,
                                      0);

    if (ERROR_SUCCESS == last_error)
    {
        BYTE exe_path[] = "C:\\path\\to\\your\\application.exe";
        DWORD last_error;
        const DWORD types_supported = EVENTLOG_ERROR_TYPE   |
                                      EVENTLOG_WARNING_TYPE |
                                      EVENTLOG_INFORMATION_TYPE;

        last_error = RegSetValueEx(key,
                                   "EventMessageFile",
                                   0,
                                   REG_SZ,
                                   exe_path,
                                   sizeof(exe_path));

        if (ERROR_SUCCESS == last_error)
        {
            last_error = RegSetValueEx(key,
                                       "TypesSupported",
                                       0,
                                       REG_DWORD,
                                       (LPBYTE) &types_supported,
                                       sizeof(types_supported));
        }

        if (ERROR_SUCCESS != last_error)
        {
            std::cerr << "Failed to install source values: "
                << last_error << "\n";
        }

        RegCloseKey(key);
    }
    else
    {
        std::cerr << "Failed to install source: " << last_error << "\n";
    }
}

void log_event_log_message(const std::string& a_msg,
                           const WORD         a_type,
                           const std::string& a_name)
{
    DWORD event_id;

    switch (a_type)
    {
        case EVENTLOG_ERROR_TYPE:
            event_id = MSG_ERROR_1;
            break;
        case EVENTLOG_WARNING_TYPE:
            event_id = MSG_WARNING_1;
            break;
        case EVENTLOG_INFORMATION_TYPE:
            event_id = MSG_INFO_1;
            break;
        default:
            std::cerr << "Unrecognised type: " << a_type << "\n";
            event_id = MSG_INFO_1;
            break;
    }

    HANDLE h_event_log = RegisterEventSource(0, a_name.c_str());

    if (0 == h_event_log)
    {
        std::cerr << "Failed open source '" << a_name << "': " <<
            GetLastError() << "\n";
    }
    else
    {
        LPCTSTR message = a_msg.c_str();

        if (FALSE == ReportEvent(h_event_log,
                                 a_type,
                                 0,
                                 event_id,
                                 0,
                                 1,
                                 0,
                                 &message,
                                 0))
        {
            std::cerr << "Failed to write message: " <<
                GetLastError() << "\n";
        }

        DeregisterEventSource(h_event_log);
    }
}

void uninstall_event_log_source(const std::string& a_name)
{
    const std::string key_path("SYSTEM\\CurrentControlSet\\Services\\"
                               "EventLog\\Application\\" + a_name);

    DWORD last_error = RegDeleteKey(HKEY_LOCAL_MACHINE,
                                    key_path.c_str());

    if (ERROR_SUCCESS != last_error)
    {
        std::cerr << "Failed to uninstall source: " << last_error << "\n";
    }
}

int main(int a_argc, char** a_argv)
{
    const std::string event_log_source_name("my-test-event-log-source");

    install_event_log_source(event_log_source_name);

    log_event_log_message("hello, information",
                          EVENTLOG_INFORMATION_TYPE,
                          event_log_source_name);

    log_event_log_message("hello, error",
                          EVENTLOG_ERROR_TYPE,
                          event_log_source_name);

    log_event_log_message("hello, warning",
                          EVENTLOG_WARNING_TYPE,
                          event_log_source_name);

    // Uninstall when your application is being uninstalled.
    //uninstall_event_log_source(event_log_source_name);

    return 0;
}
=======================================================================================================================================

Hope this helps but consider that this approach has been deprecated as stated by @Cody Gray.
