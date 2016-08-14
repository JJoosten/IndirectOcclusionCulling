#pragma once

#include <cfc/base.h>
#include "stl_lambda0.hpp"

CFC_NAMESPACE3(cfc, core, threading)

class _imp_invoker;
class _imp_kernelsemaphore;									// kernelsemaphore and lightsemaphore are based on Jeff Preshing's implementation
class _imp_lightsemaphore;
class _imp_benaphore;

class CFC_API invoker
{
public:
	invoker();
	~invoker();

	void ExecuteAll();
	void ExecuteOne();
	void Add(const stl_lambda0<>& func);
protected:
	_imp_invoker *m_impl;
};

class CFC_API mutex
{
public:
	mutex();
	~mutex();

	bool TryLock();
	void Lock();
	void Unlock();
protected:
	char m_impl[80];
	mutex(const mutex& o) {}
	void operator = (const mutex& o) {}
};

// TODO: seems not to be thread safe, need to investigate
class CFC_API spinlock
{
public:
	spinlock();
	~spinlock();

	bool TryLock();
	void Lock();
	void Unlock();
protected:
	spinlock(const spinlock& o) {}
	void operator = (const spinlock& o) {}
protected:
	char m_impl[8];
};

class CFC_API thread
{
public:
	static void CreateThreadDetached(const stl_lambda0<>& func);
	static unsigned int GetHardwareThreadCount();
	static size_t GetCurrentThreadID();
};

class CFC_API kernelsemaphore
{
public:
	kernelsemaphore();
	kernelsemaphore(unsigned int initialCount);
	~kernelsemaphore();

	void Signal(unsigned int numSignals = 1);
	void Wait();
protected:
	kernelsemaphore(const kernelsemaphore& o) {}
	void operator = (const kernelsemaphore& o) {}
private:
	void construct(unsigned int initialCount);
private:
	_imp_kernelsemaphore* m_semaphore;
}; 

class CFC_API lightsemaphore
{
public:
	lightsemaphore();
	lightsemaphore(unsigned int initialCount);
	~lightsemaphore();

	void Signal(unsigned int signalCount = 1);
	void Wait();
	bool TryWait();
protected:
	lightsemaphore(const lightsemaphore& o) {}
	void operator = (const lightsemaphore& o) {}
private:
	void WaitInternal();

private:
	_imp_lightsemaphore* m_semaphore;
};
	
typedef kernelsemaphore semaphore;

// mutex (critical section) that works using a spinlock and a semaphore. 
// can be used across threads, unlike a mutex, which always must be locked and unlocked on the same thread.
class CFC_API benaphore
{
public:
	benaphore();
	~benaphore();
	bool TryLock();
	void Lock();
	void Unlock();

protected:
	_imp_benaphore* m_impl;
};

template <class T>
class scopedlock
{
public:
	scopedlock(T& lck) : m_lock(lck) { m_lock.Lock(); }
	~scopedlock() { m_lock.Unlock(); }
protected:
	scopedlock(const scopedlock<T>& x) {}
	void operator = (const scopedlock<T>& x) {}
	T& m_lock;
};
	
class CFC_API atomic_int
{
public:
	atomic_int();
	explicit atomic_int(int v);
	~atomic_int();
	atomic_int(const atomic_int& o);
	void operator = (const atomic_int& o);

	int load() const;
	int fetch_add(int v);
	int fetch_and(int v);
	int fetch_or(int v);
	int fetch_xor(int v);

	int exchange(int v);
	void store(int v);
protected:
	volatile char m_impl[8];
};

CFC_END_NAMESPACE3(cfc, core, threading)