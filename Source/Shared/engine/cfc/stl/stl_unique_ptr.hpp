#pragma once

#include <memory>

template <class T> using stl_unique_ptr = std::unique_ptr<T>;

#if 0
template<class T>
class stl_unique_ptr
{
public:
	inline stl_unique_ptr(T* v = 0) { m_v = v; }
	inline stl_unique_ptr(const stl_unique_ptr<T>& o) { stl_unique_ptr<T>& unconst = (stl_unique_ptr<T>&)o; m_v = unconst.m_v; unconst.m_v = 0; } // steal m_v from other
	inline ~stl_unique_ptr() { _release(); }
	inline stl_unique_ptr<T>& operator = (T* v) { _acquire(v); return *this; }
	inline stl_unique_ptr<T>& operator = (const stl_unique_ptr<T>& o) { stl_unique_ptr<T>& unconst = (stl_unique_ptr<T>&)o; _acquire(unconst.m_v); unconst.m_v = 0; return *this; } // steal m_v from other
	inline operator T*() { return m_v; }
	inline T* operator ->() { return m_v; }
	inline operator T*() const { return m_v; }
	inline T* operator ->() const { return m_v; }
	inline T& operator [](int idx) { return m_v[idx]; }
	inline const T& operator [](int idx) const { return m_v[idx]; }
	inline bool operator < (T* o) const { return m_v < o; }
	inline bool operator < (const stl_unique_ptr<T>& o) const { return m_v < o.m_v; }

	// returns raw pointer
	inline T* get() { return m_v; }
	// releases pointer from stl_unique_ptr custody for manual memory management purposes
	inline T* release() { T* ret = m_v; m_v = 0; return ret; }
protected:
	inline void _acquire(T* ptr) { if (ptr != m_v) _release(); m_v = ptr; }
	inline void _release() {
		if (m_v) {
			delete m_v; 
			m_v = 0;
		}
	}
	T* m_v;
};
#endif