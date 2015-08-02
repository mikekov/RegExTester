#pragma once
#include "basetypes.h"

namespace mcr {


class CSyncObject
{
public:
	virtual bool Lock(DWORD timeout= INFINITE);
	virtual bool Unlock() = 0;
	virtual bool Unlock(LONG count, LONG* prev_count= 0);

protected:
	CSyncObject() {}
};


class CCriticalSection : public CSyncObject
{
public:
	CCriticalSection()	{ ::InitializeCriticalSection(&cs_); }
	~CCriticalSection()	{ ::DeleteCriticalSection(&cs_); }

	virtual bool Lock(); // 	{ ::EnterCriticalSection(&cs_); }
	virtual bool Unlock();	//	{ ::LeaveCriticalSection(&cs_); }

private:
	CRITICAL_SECTION cs_;
};


class CSingleLock
{
public:
	explicit CSingleLock(CSyncObject* object);
	
	CSingleLock(CSyncObject* object, bool initially_locked);

	~CSingleLock();

private:
	CSyncObject* object_;
};


} // namespace
