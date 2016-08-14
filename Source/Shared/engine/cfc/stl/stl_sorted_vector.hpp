#pragma once

#include "stl_algorithm.hpp"

namespace stl_sorted_vector
{
	template <class Vector, class T>
	typename Vector::iterator insert(Vector& v, const T& t) {
		typename Vector::iterator i = std::lower_bound(v.begin(), v.end(), t);
		if (i == v.end() || t < *i)
			return v.insert(i, t);
		else
			return i;
	}

	template <class Vector, class T>
	typename Vector::iterator insert_or_overwrite(Vector& v, const T& t) {
		typename Vector::iterator i = std::lower_bound(v.begin(), v.end(), t);
		if (i == v.end() || t < *i)
			return v.insert(i, t);
		else
		{
			(*i) = t;
			return i;
		}
	}

	template <class Vector, class T>
	void insert_multikey(Vector& v, const T& t) {
		typename Vector::iterator i = std::lower_bound(v.begin(), v.end(), t);
		v.insert(i, t);
	}


	template <class Vector, class T>
	typename Vector::iterator find(Vector& v, const T& t) {
		typename Vector::iterator i = std::lower_bound(v.begin(), v.end(), t);
		return i == v.end() || (t < *i) ? v.end() : i;
	}

	template <class Vector, class T, class Cmp>
	void pred_insert_multikey(Vector& v, const T& t, Cmp pred) {
		typename Vector::iterator i = std::lower_bound(v.begin(), v.end(), t, pred);
		v.insert(i, t);
	}

	template <class Vector, class T, class Cmp>
	void pred_insert(Vector& v, const T& t, Cmp pred) {
		typename Vector::iterator i = std::lower_bound(v.begin(), v.end(), t, pred);
		if (i == v.end() || pred(t, *i))
			v.insert(i, t);
	}

	template <class Vector, class T, class Cmp>
	typename Vector::iterator pred_find(Vector& v, const T& t, Cmp pred) {
		typename Vector::iterator i = std::lower_bound(v.begin(), v.end(), t, pred);
		return i == v.end() || pred(t, *i) ? v.end() : i;
	}

	template <class Vector, class T>
	bool erase(Vector& v, const T& t) {
		typename Vector::iterator i = find(v, t);
		if (i != v.end())
		{
			v.erase(i);
			return true;
		}
		else
			return false;
	}

}
