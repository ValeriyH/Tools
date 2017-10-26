#include "stdafx.h"
#include "LogManager.h"
#include <Windows.h>
#include <string>

LogManager::LogManager()
{
    _logger = new NoLogger();
}


LogManager::~LogManager()
{
    delete _logger;
}

LogManager & LogManager::_instance()
{
    static LogManager manager;
    return manager;
}

//Returns handle of currently executed module (.dll or .exe)
HMODULE GetCurrentModule()
{ // NB: XP+ solution!
    HMODULE hModule = NULL;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCTSTR)GetCurrentModule,
        &hModule);

    return hModule;
}

bool LogManager::SetLogger(const char * name)
{
    HMODULE hInstLibrary = GetCurrentModule();
    if (!hInstLibrary) return false;

    std::string strAllocFunction("Create");
    strAllocFunction += name;

    ILogger *(*alloc_fun)() = (ILogger *(*)())GetProcAddress(hInstLibrary, strAllocFunction.c_str());
    ILogger *logger_ptr = alloc_fun ? alloc_fun() : NULL; 
    
    if (logger_ptr)
    {
        //TODO Add locks for multithreads
        ILogger *tmp = _instance()._logger;
        _instance()._logger = logger_ptr;
        delete tmp;
        return true;
    }
    //TODO else try to loadlibrary <name>.dll and get ILogger there
    return false;
}

void LogManager::Log(int code, const char * message)
{
    //TODO this one should add message to queue, processed by separate thread
    //So ILogger::Log sholdn't care about multithread
    _instance()._logger->Log(code, message);
}
