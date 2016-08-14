#define CFC_CONF_CPP11_THREADING

#include "threading.h"
#include <vector>

// CPP11 includes
#ifdef CFC_CONF_CPP11_THREADING
#	include <mutex>
#	include <atomic>
#	include <thread>
#endif

namespace cfc {
namespace core {
namespace threading {

#pragma region Mutex
#ifdef CFC_CONF_CPP11_THREADING
#define CFC_GET_MUTEX() ((std::mutex*)m_impl)
#endif

mutex::mutex()
{
#ifdef CFC_CONF_CPP11_THREADING
	static_assert (sizeof(std::mutex) <= sizeof(m_impl) , "implementation size is not sufficient for mutex");

	new (CFC_GET_MUTEX()) std::mutex();

#endif
}

mutex::~mutex()
{
#ifdef CFC_CONF_CPP11_THREADING
	CFC_GET_MUTEX()->std::mutex::~mutex();
#endif
}

bool mutex::TryLock()
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_MUTEX()->try_lock();
#endif
}

void mutex::Lock()
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_MUTEX()->lock();
#endif
}

void mutex::Unlock()
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_MUTEX()->unlock();
#endif
}

#ifdef CFC_CONF_CPP11_THREADING
#undef CFC_GET_MUTEX
#endif
#pragma endregion

#pragma region Spinlock
#ifdef CFC_CONF_CPP11_THREADING
#define CFC_GET_ATOMIC_BOOL() ((std::atomic<bool>*)m_impl)
#endif 

spinlock::spinlock()
{
#ifdef CFC_CONF_CPP11_THREADING
	static_assert (sizeof(std::atomic<bool>) <= sizeof(m_impl), "implementation size is not sufficient for atomic");

	new (CFC_GET_ATOMIC_BOOL()) std::atomic<bool>();
#endif
}

spinlock::~spinlock()
{
	// when the destructor is called and we are still locked, BREAK, this is not intended behaviour!
	if (*CFC_GET_ATOMIC_BOOL()==true) CFC_BREAKPOINT;

#ifdef CFC_CONF_CPP11_THREADING
	CFC_GET_ATOMIC_BOOL()->std::atomic<bool>::~atomic();
#endif
}

bool spinlock::TryLock()
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_ATOMIC_BOOL()->exchange(true) == false;
#endif
}

void spinlock::Lock()
{
#ifdef CFC_CONF_CPP11_THREADING
	while (CFC_GET_ATOMIC_BOOL()->exchange(true) == false) { /* spin here */ }
#endif
}

void spinlock::Unlock()
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_ATOMIC_BOOL()->store(false);
#endif
}

#ifdef CFC_CONF_CPP11_THREADING
#undef CFC_GET_ATOMIC_BOOL
#endif

#pragma endregion
#pragma region Invoker

class _imp_invoker
{
public:
	struct fpObject
	{
		stl_lambda0<> func;
	};

	mutex m_invokeLock;
	std::vector<fpObject> m_queue;
	std::vector<fpObject> m_queueBack;
	int m_queueBackIndex;
};


invoker::invoker()
{
	m_impl = new _imp_invoker();
	m_impl->m_queueBackIndex = 0;
}

invoker::~invoker()
{
	delete m_impl;
}

void invoker::Add(const stl_lambda0<>& func)
{
	_imp_invoker::fpObject obj = { func };
	m_impl->m_invokeLock.Lock();
	m_impl->m_queue.push_back(obj);
	m_impl->m_invokeLock.Unlock();
}

void invoker::ExecuteAll()
{
	// fetch data
	if (m_impl->m_queue.size() == 0)
		return;

	std::vector<_imp_invoker::fpObject> queueSelf;
	m_impl->m_invokeLock.Lock();
	m_impl->m_queue.swap(queueSelf);
	m_impl->m_invokeLock.Unlock();

	for (auto it : queueSelf)
	{
		it.func();
	}

}


void invoker::ExecuteOne()
{
	// early out
	if (m_impl->m_queue.size() == 0)
		return;

	_imp_invoker::fpObject obj;
	{
		scopedlock<mutex> invokeLock(m_impl->m_invokeLock);

		// another time, this time safely
		if (m_impl->m_queue.size() == 0)
			return;

		obj = m_impl->m_queue.front();

		// TODO: erasing first element in a std::vector is very inefficient when there are a large quantity of objects in the vector - change container type when this becomes a problem
		m_impl->m_queue.erase(m_impl->m_queue.begin());
	}

	obj.func();

}


#pragma endregion
#pragma region Thread

void thread::CreateThreadDetached(const stl_lambda0<>& func)
{
#ifdef CFC_CONF_CPP11_THREADING
	std::thread th([func]() { func(); });
	th.detach();
#else 
#error threading not implemented for this compiler
#endif
}

unsigned int thread::GetHardwareThreadCount()
{
#ifdef CFC_CONF_CPP11_THREADING
	// returns 0 when this can not be computed correctly or when this function is not well defined
	const unsigned int hardwareConcurrency = std::thread::hardware_concurrency();
	if(hardwareConcurrency == 0) CFC_BREAKPOINT;
	return hardwareConcurrency;
#else 
#error threading not implemented for this compiler
#endif
}

size_t thread::GetCurrentThreadID()
{
#ifdef CFC_CONF_CPP11_THREADING 
	//static_assert(sizeof(std::thread::id)<=sizeof(size_t),"this function only works if size of thead::id is equal or smaller than the size of size_t");
	auto id = std::this_thread::get_id();
	return *(size_t*)&id;
#else 
#error threading not implemented for this compiler
#endif
}

#pragma endregion
#pragma region Kernel Semaphore
// semaphore and lightSemaphore are based on Jeff Preshing implementation

#if defined(_WIN32) || defined(__MACH__) || defined(__unix__)

#ifdef _WIN32
	#include <windows.h>
#elif __MACH__
	#include <mach/mach.h>
#elif __unix__
	#include <semaphore.h>
#endif

class _imp_kernelsemaphore
{
public:
#ifdef _WIN32
	HANDLE m_handle;
#elif __MACH__
	semaphore_t m_handle;
#elif __unix__
	sem_t m_handle;
#endif
};

kernelsemaphore::kernelsemaphore()
	:
m_semaphore(NULL)
{
	construct(0);
}

kernelsemaphore::kernelsemaphore(unsigned int initialCount /*= 0*/)
	:
m_semaphore(NULL)
{
	construct(initialCount);
}

void kernelsemaphore::construct(unsigned int initialCount)
{
	m_semaphore = new _imp_kernelsemaphore();

#ifdef _WIN32
	m_semaphore->m_handle = CreateSemaphore(NULL, 0, MAXLONG, NULL);
#elif __MACH__
	semaphore_create(mach_task_self(), &m_semaphore->m_handle, SYNC_POLICY_FIFO, 0);
#elif __unix__
	sem_init(&m_semaphore->m_handle, 0, initialCount);
#endif
}

kernelsemaphore::~kernelsemaphore()
{
#ifdef _WIN32
	CloseHandle(m_semaphore->m_handle);
#elif __MACH__
	semaphore_destroy(mach_task_self(), m_semaphore->m_handle);
#elif __unix__
	sem_destroy(&m_semaphore->m_handle);
#endif

	delete m_semaphore;
}

void kernelsemaphore::Signal(unsigned int numSignals /*= 1*/)
{
	if (numSignals < 1) CFC_BREAKPOINT;

#ifdef _WIN32
	ReleaseSemaphore(m_semaphore->m_handle, numSignals, NULL);
#elif __MACH__
	while (numSignals-- > 0)
		semaphore_signal(m_semaphore->m_handle);
#elif __unix__
	while (numSignals-- > 0)
		sem_post(&m_semaphore->m_handle);
#endif
}

void kernelsemaphore::Wait()
{

#ifdef _WIN32
	WaitForSingleObject(m_semaphore->m_handle, INFINITE);
#elif __MACH__
	semaphore_wait(m_semaphore->m_handle);
#elif __unix__
	// http://stackoverflow.com/questions/2013181/gdb-causes-sem-wait-to-fail-with-eintr-error
	int rc;
	do
	{
		rc = sem_wait(&m_semaphore->m_handle);
	} while (rc == -1 && errno == EINTR);
#endif
}
#endif // ifdef _WIN32 || __MACH__ || __unix__


#pragma endregion
#pragma region Light(weight) Semaphore

class _imp_lightsemaphore
{
public:
	_imp_lightsemaphore(unsigned int initialCount)
		:
	m_count(initialCount)
	, m_handle(initialCount)
	{

	}

public:
	std::atomic<int> m_count;
	kernelsemaphore m_handle;
};

lightsemaphore::lightsemaphore()
	:
m_semaphore(NULL)
{
	m_semaphore = new _imp_lightsemaphore(0);
}

lightsemaphore::lightsemaphore(unsigned int initialCount /*= 0*/)
	:
m_semaphore(NULL)

{
	m_semaphore = new _imp_lightsemaphore(initialCount);
}

lightsemaphore::~lightsemaphore()
{
	delete m_semaphore;
}

void lightsemaphore::Signal(unsigned int numSignals)
{
	if (numSignals < 1) CFC_BREAKPOINT;

	int oldCount = m_semaphore->m_count.fetch_add(numSignals, std::memory_order_release);
	int toRelease = -oldCount < (int)numSignals ? -oldCount : (int)numSignals;
	if (toRelease > 0)
	{
		m_semaphore->m_handle.Signal(toRelease);
	}
}

void lightsemaphore::Wait()
{
	if (!TryWait())
		WaitInternal();
}

bool lightsemaphore::TryWait()
{
	int oldCount = m_semaphore->m_count.load(std::memory_order_relaxed);
	return (oldCount > 0 && m_semaphore->m_count.compare_exchange_strong(oldCount, oldCount - 1, std::memory_order_acquire));
}

void lightsemaphore::WaitInternal()
{
	int oldCount;

	int initialSpin = 10000;
	while (initialSpin--)
	{
		oldCount = m_semaphore->m_count.load(std::memory_order_relaxed);
		
		if ((oldCount > 0) && m_semaphore->m_count.compare_exchange_strong(oldCount, oldCount - 1, std::memory_order_acquire))
			return;

		std::atomic_signal_fence(std::memory_order_acquire);     // Prevent the compiler from collapsing the loop.
	}

	oldCount = m_semaphore->m_count.fetch_sub(1, std::memory_order_acquire);

	if (oldCount <= 0)
		m_semaphore->m_handle.Wait();
}

#pragma endregion
#pragma region Benaphore
class _imp_benaphore
{
public:
	std::atomic<int> m_contentionCount;
	kernelsemaphore m_sema;
};

benaphore::benaphore()
{
	m_impl = new _imp_benaphore();
}

benaphore::~benaphore()
{
	delete m_impl;
}

bool benaphore::TryLock()
{
	if (m_impl->m_contentionCount.load(std::memory_order_relaxed) != 0)
		return false;
	int expected = 0;
	return m_impl->m_contentionCount.compare_exchange_strong(expected, 1, std::memory_order_acquire);
}

void benaphore::Lock()
{
	if (m_impl->m_contentionCount.fetch_add(1, std::memory_order_acquire) > 0)
	{
		m_impl->m_sema.Wait();
	}
}

void benaphore::Unlock()
{
	int oldCount = m_impl->m_contentionCount.fetch_sub(1, std::memory_order_release);
	if (oldCount <= 0)
		CFC_BREAKPOINT; // should not occur.
	if (oldCount > 1)
	{
		m_impl->m_sema.Signal();
	}

}

#pragma endregion

#pragma region Atomics

#ifdef CFC_CONF_CPP11_THREADING
# define CFC_GET_ATOMIC() ((std::atomic<int>*)(m_impl))
#endif

atomic_int::atomic_int()
{
#ifdef CFC_CONF_CPP11_THREADING
	static_assert (sizeof(std::atomic<int>) <= sizeof(m_impl), "implementation size is not sufficient for atomic");

	new (CFC_GET_ATOMIC()) std::atomic<int>();
#endif
}


atomic_int::atomic_int(int v)
{
#ifdef CFC_CONF_CPP11_THREADING
	static_assert (sizeof(std::atomic<int>) <= sizeof(m_impl), "implementation size is not sufficient for atomic");

	new (CFC_GET_ATOMIC()) std::atomic<int>();
#endif
	store(v);
}

atomic_int::~atomic_int()
{
#ifdef CFC_CONF_CPP11_THREADING
	CFC_GET_ATOMIC()->std::atomic<int>::~atomic();
#endif
}

atomic_int::atomic_int(const atomic_int& o)
{
#ifdef CFC_CONF_CPP11_THREADING
	static_assert (sizeof(std::atomic<int>) <= sizeof(m_impl), "implementation size is not sufficient for atomic");
	new (CFC_GET_ATOMIC()) std::atomic<int>();
#endif

	store(o.load());
}

void atomic_int::operator=(const atomic_int& o)
{
	store(o.load());
}

int atomic_int::load() const
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_ATOMIC()->load();

#endif
}

int atomic_int::fetch_add(int v)
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_ATOMIC()->fetch_add(v);
#endif
}

int atomic_int::fetch_and(int v)
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_ATOMIC()->fetch_and(v);
#endif

}

int atomic_int::fetch_or(int v)
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_ATOMIC()->fetch_or(v);
#endif

}

int atomic_int::fetch_xor(int v)
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_ATOMIC()->fetch_xor(v);
#endif
}

int atomic_int::exchange(int v)
{
#ifdef CFC_CONF_CPP11_THREADING
	return CFC_GET_ATOMIC()->exchange(v);
#endif
}

void atomic_int::store(int v)
{
#ifdef CFC_CONF_CPP11_THREADING
	CFC_GET_ATOMIC()->store(v);
#endif
}

#ifdef CFC_CONF_CPP11_THREADING
# undef CFC_GET_ATOMIC
#endif

#pragma endregion


};
};
};