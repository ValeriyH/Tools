#pragma once
#include <Windows.h>

class Injector
{
public:
    Injector();
    virtual ~Injector();

    static void InjectDll(DWORD procID);
};

