#pragma once

#include "stl_common.hpp"
#include "stl_string.hpp"

class STL_API stl_string_advanced
{
public:
	static stl_string				replace(const stl_string& target, const stl_string& find, const stl_string& replaceBy);
	template <class T> static void	split_foreach(const stl_string& target, const char* seperator, const T& clb) { split_raw(target, seperator, [](const char* a, size_t b, void* c) { const T& clb = *(const T*)c; clb(a,b); }, (void*)&clb); }
	static int						split_array(const stl_string& target, const char* seperator, stl_string* outArray, int outArrayMaxSize);

	static stl_string				sprintf(const stl_string format, ...);

	static void						utf8_push_back(stl_string& target, unsigned int codepoint);
	static void						utf8_pop_back(stl_string& target);
	template <class T> static bool	utf8_foreach(const stl_string& target, const T& clb) { return utf8_iterate_raw(target, [](unsigned int a, void* b) { const T& clb = *(const T*)b; clb(a); }, (void*)&clb); }
	static u32_64					utf8_length(const stl_string& target);
	static stl_string				utf8_fromUtf16(const u16* utf16, int numShorts);
	
	static stl_string				utf16_fromUtf8(const stl_string& target);
	static void						utf16_push_back(stl_string& target, unsigned int codepoint);
protected:
	static void split_raw(const stl_string& target, const char* seperator, void(*)(const char* buffer, size_t length, void* custom), void* custom);
	static bool utf8_iterate_raw(const stl_string& target, void(*)(unsigned int codepoint, void* custom), void* custom);
};
