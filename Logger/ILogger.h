#pragma once

//Logger interface. Header should be included to all loggers declarations
class ILogger
{
public:
    virtual void Log(int severityCode, const char* message) = 0;
    //Config file, Regestry path, config XML and etc. By default or if not need NULL.
    virtual void SetConfig(const char* configuration) {};
    virtual ~ILogger() {};
};


class NoLogger : public ILogger
{
public:
    virtual void Log(int severityCode, const char* message)
    {
    }
};
