//List of logggers
//Them can be added to current process or be in separate dll

#include "stdafx.h"
#include "ILogger.h"
#include <string>  


class ConsoleLogger : public ILogger
{

    virtual void Log(int severityCode, const char* message)
    {
        printf("[%d]%s\n",severityCode, message);
    }
};

extern "C" __declspec(dllexport) ILogger *CreateConsoleLogger() { return new ConsoleLogger(); }