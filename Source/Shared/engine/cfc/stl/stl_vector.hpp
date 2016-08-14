#pragma once

// STL implementation
#include <vector>

template <typename T> using stl_vector = std::vector<T>;

template <typename T> auto stl_vector_push_back_n(T& vec, usize N) -> decltype(&vec.at(0))
{
	usize pos = vec.size();
	vec.resize(pos + N);
	return &vec.at(pos);
}