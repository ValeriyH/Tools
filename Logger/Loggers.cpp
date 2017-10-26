#include "stdafx.h"
#include "ILogger.h"
#include <string>  

//ILogger implementation
//It can be added to current process or be in separate dll
class ConsoleLogger : public ILogger
{

    virtual void Log(int severityCode, const char* message)
    {
        printf("[%d]%s\n",severityCode, message);
    }
};

extern "C" __declspec(dllexport) ILogger *CreateConsoleLogger() { return new ConsoleLogger(); }


class FileLogger : public ILogger
{

    virtual void Log(int severityCode, const char* message)
    {
        printf("<FILE>[%d]%s\n", severityCode, message);
    }
};

extern "C" __declspec(dllexport) ILogger *CreateFileLogger() { return new FileLogger(); }