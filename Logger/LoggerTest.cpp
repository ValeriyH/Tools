// Logger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LogManager.h"

int main()
{
    LogManager::SetLogger("ConsoleLogger");
    LogManager::Log(10, "Hello World!");
    if (!LogManager::SetLogger("ConsoleLogger2"))
    {
        LogManager::Log(20, "Logger doesn't changed!");
    }
    LogManager::SetLogger("FileLogger");
    LogManager::Log(30, "Hello World again!");

    return 0;
}

