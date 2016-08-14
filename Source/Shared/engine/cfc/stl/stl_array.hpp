#pragma once

#include "stl_common.hpp"

template <class T, usize Size>
class stl_array
{
public:
	T& operator [] (usize v) { stl_assert(v<Size); return arr[v];}
	const T& operator [] (usize v) const { stl_assert(v<Size); return arr[v];}

	usize capacity() const { return Size; }
	usize size() const { return Size; }
protected:
	T arr[Size];
};

template <class T, usize Size>
class stl_static_vector
{
public:
	T& operator [] (usize v) { stl_assert(v<count); return arr[v];}
	const T& operator [] (usize v) const { stl_assert(v<count); return arr[v];}

	void push_back(const T& elem) { stl_assert(count < Size); arr[count++] = v;}
	void resize(usize sz) { stl_assert(sz <= Size); count = sz; }
	usize size() const { return count; }
	usize capacity() const { return Size; }
	T* begin() const { return arr; }
	T* end() const { return arr+count; }
protected:
	T arr[Size];
	usize count=0;
};

namespace stl_arbitrary_array
{
	template <class It, class T>
	void insert(It i, It end, const T& t)
	{
		while (i != end)
		{
			It vc = end;
			(*vc) = *(--end);
		}
		*i = t;
	}

	template <class It>
	void erase(It i, It end)
	{
		--end;
		while (i != end)
		{
			It vc = i;
			(*vc) = *(++i);
		}
	}

	template <class It>
	void swap_erase(It i, It end)
	{
		--end;
		*i = *end;
	}
}