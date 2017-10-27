#pragma once

#include "windows.h"

class RWLockException
    : public std::exception
{
public:
    RWLockException(int code, char* message)
        :exception(message)
    {
        error = code;
    }

    int error;
};

class RWLock
{
public:
    RWLock();
    virtual ~RWLock();

    void ReadLock();
    void ReadUnlock();
	//NOTE: WriteLock is exclusive lock. 
    //Thread shouldn't have readlocks if it requested write lock. But it is ok to get write lock and then get read lock
    //throw exception if thread have read locks RWLockException(ERROR_LOCKED) or timed out RWLockException(WAIT_TIMEOUT)
    void WriteLock(int timeout = INFINITE) throw (RWLockException);
    void WriteUnlock();

private:
    int getReadLocks();
    void setReadLocks(int value);

    CRITICAL_SECTION _lock;
    DWORD _readLocks;
    long _readers;
};

template<class T>
class RWLockT
    : public T
    , public RWLock
{

};