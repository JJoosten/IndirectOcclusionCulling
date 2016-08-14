#pragma once

#include "stl_common.hpp"

template <class T, int size> class stl_pimpl
{
public:
	stl_pimpl() : m_impl(nullptr) {}

	~stl_pimpl() 
	{ 
		stl_assert(m_impl == nullptr);	// object needs to be destroyed
	}

	inline T* operator ->() const
	{
		stl_assert(m_impl != nullptr);				// need to have a valid object
		return m_impl; 
	}

	template <bool dummy=false> void init()
	{ 
		static_assert(sizeof(T) <= sizeof(m_buffer), "pimpl buffer is too small to initialize object"); // data buffer needs to be large enough to fit data
		stl_assert(m_impl == nullptr);	// there can't be a valid object yet
		
		m_impl = new (m_buffer) T(); 
	}

	template<bool dummy=false> void destroy()		
	{ 
		stl_assert(m_impl != nullptr);				// needs to have a valid object

		m_impl->~T(); 
		m_impl = nullptr;
	}

	T* get()
	{
		stl_assert(m_impl != nullptr);				// need to have a valid object
		return m_impl;
	}
protected:
	char m_buffer[size - sizeof(T*)];
	T* m_impl;
};