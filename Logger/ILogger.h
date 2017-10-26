#pragma once

//Logger interface. Header should be included to all loggers declarations
class ILogger
{
public:
    virtual void Log(int severityCode, const char* message) = 0;
    virtual ~ILogger() {};
};


class NoLogger : public ILogger
{
public:
    virtual void Log(int severityCode, const char* message)
    {
    }
};
