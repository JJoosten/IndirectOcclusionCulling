#pragma once

#include "stl_algorithm.hpp"
#include "stl_array.hpp"

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

namespace stl_sorted_array
{
	template <class It, class T>
	bool insert(It begin, It end, const T& t) {
		It i = std::lower_bound(begin, end, t);
		if (i == end || t < *i)
		{
			stl_arbitrary_array::insert(i, end, t);
			return true;
		}
		else
			return false;
	}

	template <class It, class T>
	bool erase(It begin, It end, const T& t) {
		It i = std::lower_bound(begin, end, t);
		if (i == end || t < *i)
		{
			return false;
		}
		else
		{
			array::erase(i, end);
			return true;
		}
	}

	template <class It, class T>
	It insert_or_overwrite(It begin, It end, const T& t) {
		It i = std::lower_bound(begin, end, t);
		if (i == end || t < *i)
		{
			stl_arbitrary_array::insert(i, end, t);
			return i;
		}
		else
		{
			(*i) = t;
			return i;
		}
	}

	template <class It, class T>
	It insert_multikey(It begin, It end, const T& t) {
		It i = std::lower_bound(begin, end, t);
		stl_arbitrary_array::insert(i, end, t);
		return i;
	}


	template <class It, class T>
	It find(It begin, It end, const T& t) {
		It i = std::lower_bound(begin, end, t);
		return i == end || (t < *i) ? end : i;
	}
}
