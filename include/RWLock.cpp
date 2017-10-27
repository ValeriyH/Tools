#include "stdafx.h"
#include "RWLock.h"


RWLock::RWLock()
{
    _readers = 0;
    InitializeCriticalSection(&_lock);
    _readLocks = TlsAlloc();
}

RWLock::~RWLock()
{
    DeleteCriticalSection(&_lock);
    _readers = 0;
    TlsFree(_readLocks);
}

void RWLock::ReadLock()
{
    int locks = getReadLocks() + 1;

    if (locks > 1)
    {
        //If thread already have readlock don't wait for critical section, this section can be already blocked by write lock.
        InterlockedIncrement(&_readers);
    }
    else
    {
        EnterCriticalSection(&_lock);
        InterlockedIncrement(&_readers);
        LeaveCriticalSection(&_lock);
    }
    setReadLocks(locks);
}

void RWLock::ReadUnlock()
{
    InterlockedDecrement(&_readers);
    int locks = getReadLocks() - 1;
    setReadLocks(locks);
}

void RWLock::WriteLock(int timeout /*= INFINITE*/)
{
	//NOTE: Thread shouldn't have readlocks if it requested write lock
    int locks = getReadLocks();
    if (locks)
    {
        throw RWLockException(ERROR_LOCKED, "Thread already have read-lock. The write-lock is unacceptable");
    }

    EnterCriticalSection(&_lock);
    DWORD dwStart = GetTickCount();
    while (InterlockedCompareExchange(&_readers, 0, 0))
    {
        Sleep(0);
        DWORD elapsed = 0;
        DWORD ticks = GetTickCount();
        if (ticks >= dwStart)
        {
            elapsed = ticks - dwStart;
        }
        else
        {
            /* GetTickCount resets after 49 days */
            elapsed = MAXDWORD - dwStart + ticks;
        }

        if (timeout > 0 && elapsed > timeout)
        {
            //Failed to aquire section in defined time
            LeaveCriticalSection(&_lock);
            throw RWLockException(WAIT_TIMEOUT, "Thread already have read-lock. The write-lock is unacceptable");
        }
    }
}

void RWLock::WriteUnlock()
{
    LeaveCriticalSection(&_lock);
}

//Get read locks by thread
int RWLock::getReadLocks()
{
    return reinterpret_cast<unsigned int>(TlsGetValue(_readLocks));;
}

//Set read locks by thread
void RWLock::setReadLocks(int value)
{
    TlsSetValue(_readLocks, reinterpret_cast<LPVOID>(value));
}
