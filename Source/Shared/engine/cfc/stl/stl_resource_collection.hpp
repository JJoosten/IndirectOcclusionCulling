#pragma once

#include "stl_common.hpp"
#include "stl_vector.hpp"

#include <concurrent_vector.h>

template <class T, class Lock=stl_no_mutex, class Key = usize>
class stl_resource_collection
{
public:
	usize insert() 
	{ 
		m_lock.lock();
		if (m_freelist.empty())
		{			
			m_lock.unlock();

			// return new object
			auto it = m_objects.push_back(T());
			return it-m_objects.begin();
		}

		// return from freelist
		Key idx = m_freelist.back();
		m_freelist.pop_back();
		m_lock.unlock();
		return idx;
	}
	void erase(usize index)
	{
		stl_assert(index < m_objects.size());
		m_objects[index] = T();
		m_lock.lock();
		m_freelist.push_back(index);
		m_lock.unlock();
	}

	T& operator [] (usize index) { stl_assert(index < m_objects.size()); return m_objects[index]; }
	const T& operator [] (usize index) const { stl_assert(index < m_objects.size()); return m_objects[index]; }

	auto begin() { return m_objects.begin(); }
	auto end()  { return m_objects.end(); }
protected:
	Lock m_lock;
	concurrency::concurrent_vector<T> m_objects;
	stl_vector<Key> m_freelist;
};