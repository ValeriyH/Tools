#pragma once

#include "ILogger.h"

//Singleton logger manager.
class LogManager
{
public:
    static bool SetLogger(const char* name, const char* config = NULL);
    
    static void Log(int code, const char* message);

private:
    LogManager();
    ~LogManager();

    static LogManager& _instance();
    ILogger* _logger;
};

