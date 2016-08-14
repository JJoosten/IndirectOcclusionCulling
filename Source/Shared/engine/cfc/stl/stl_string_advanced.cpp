#include "stl_string_advanced.hpp"
#include "stl_unique_ptr.hpp"
#include <dependencies/utf8dec.h>

#include <stdarg.h>  // For va_start, etc.

int stl_string_advanced::split_array(const stl_string& target, const char* seperator, stl_string* outArray, int outArrayMaxSize)
{
	int i = 0;
	split_foreach(target, seperator, [outArray, outArrayMaxSize, &i](const char* part, size_t len) {
		if (i >= outArrayMaxSize)
			return;

		outArray[i].assign(part, len);
		i++;
	});
	return i;
}

stl_string stl_string_advanced::sprintf(const stl_string fmt_str, ...) {
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	stl_string str;
	stl_unique_ptr<char[]> formatted;
	va_list ap;
	while (1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		strcpy(&formatted[0], fmt_str.c_str());
		va_start(ap, fmt_str);
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return stl_string(formatted.get());
}

stl_string stl_string_advanced::replace(const stl_string& target, const stl_string& find, const stl_string& replaceBy)
{
	size_t findLength = find.size();
	size_t replaceByLength = replaceBy.size();
	const char* c = target.c_str();
	const char* cEnd = target.c_str() + target.size();
	const char* cFind = find.c_str();
	const char* cFindEnd = find.c_str() + findLength;

	stl_string strNew;
	while (c != cEnd)
	{
		if (*c == *cFind)
		{
			bool found = false;
			const char* cDetect = c;
			while (cDetect != cEnd)
			{
				if (*(cDetect++) != *(cFind++))
				{
					found = false;
					break;
				}

				if (cFind == cFindEnd)
				{
					found = true;
					break;
				}
			}

			cFind = find.c_str();

			if (found)
			{
				strNew += replaceBy;
				c = cDetect;
			}
			else
			{
				// just push the character
				strNew.push_back(*c++);
				c++;
			}
		}
		else
		{
			// unknown character
			strNew.push_back(*c++);
		}
	}

	return strNew;
}

void stl_string_advanced::utf8_push_back(stl_string& target, unsigned int codepoint)
{
	if (codepoint <= 0x7f)
		target.push_back(static_cast<char>(codepoint));
	else if (codepoint <= 0x7ff)
	{
		target.push_back(static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
		target.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
	}
	else if (codepoint <= 0xffff)
	{
		target.push_back(static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
		target.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
		target.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
	}
	else
	{
		target.push_back(static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
		target.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
		target.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
		target.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
	}
}

void stl_string_advanced::utf8_pop_back(stl_string& data)
{
	usize size = data.size();
	stl_assert(size != 0);

	if (((unsigned char)data[size - 1]) <= 0x7F)
		data.pop_back();
	else
	{
		// utf-8 pop-back
		unsigned char cc = 0;
		while (((cc = (unsigned char)data[--size]) & 0xC0) != 0xC0) { data.pop_back(); }
		data.pop_back();
	}
}

void stl_string_advanced::utf16_push_back(stl_string& target, unsigned int codepoint)
{
	if (codepoint <= 0xFFFF)
	{
		target.push_back(static_cast<char>(codepoint & 0xff));
		target.push_back(static_cast<char>((codepoint >> 8) & 0xff));
	}
	else
	{
		codepoint -= 0x10000;
		wchar_t p0 = (wchar_t)((codepoint >> 10) + 0xD800);
		target.push_back(static_cast<char>(p0 & 0xff));
		target.push_back(static_cast<char>((p0 >> 8) & 0xff));
		wchar_t p1 = (wchar_t)((codepoint & 0x3FF) + 0xDC00);
		target.push_back(static_cast<char>(p1 & 0xff));
		target.push_back(static_cast<char>((p1 >> 8) & 0xff));
	}
}



void split_threadunsafe_raw(const stl_string& target, const char* seperator, void(*callback)(const char* buffer, size_t length, void* custom), void* custom)
{
	char* c = (char*)target.c_str();
	char* cBase = c;
	char* cEnd = c + target.size();
	size_t sepLength = strlen(seperator);
	while (c < cEnd)
	{
		if (memcmp(c, seperator, sepLength) == 0)
		{
			char remembered = *c;
			*c = 0;
			callback(cBase, c-cBase, custom);
			*c = remembered;
			c += sepLength;
			cBase = c;
		}
		else
			++c;
	}
	callback(cBase, c-cBase, custom);
}

void stl_string_advanced::split_raw(const stl_string& target, const char* seperator, void(*callback)(const char* buffer, size_t length, void* custom), void* custom)
{
	stl_string v2 = target;
	split_threadunsafe_raw(v2,seperator, callback, custom);
}


bool stl_string_advanced::utf8_iterate_raw(const stl_string& target, void(*decoded)(unsigned int glyph, void* custom), void* custom)
{
	unsigned int codepoint;
	unsigned int state = UTF8_ACCEPT;
	const unsigned char* s = (const unsigned char*)target.c_str();
	const unsigned char* sEnd = (const unsigned char*)target.c_str() + target.size();

	for (; s != sEnd; ++s)
	{
		switch (utf8_decode(&state, &codepoint, *s))
		{
		case UTF8_ACCEPT:
			decoded(codepoint, custom);
			break;
		case UTF8_REJECT:
			return false;
		default:
			break;
		}
	}
	return true;
}

usize stl_string_advanced::utf8_length(const stl_string& target)
{
	usize count = 0;
	unsigned int codepoint;
	unsigned int state = UTF8_ACCEPT;
	const unsigned char* s = (const unsigned char*)target.c_str();
	const unsigned char* sEnd = (const unsigned char*)target.c_str() + target.size();

	for (; s != sEnd; ++s)
	{
		switch (utf8_decode(&state, &codepoint, *s))
		{
		case UTF8_ACCEPT:
			++count;
			break;
		case UTF8_REJECT:
			return -1;
		default:
			break;
		}
	}
	return count;
}

stl_string stl_string_advanced::utf8_fromUtf16(const u16* utf16, int numShorts)
{
	stl_string ret;
	for (int i = 0; i < numShorts; i++)
	{
		if (utf16[i] == 0)
			break;
		stl_string_advanced::utf8_push_back(ret, utf16[i]);
	}
	return ret;
}

stl_string stl_string_advanced::utf16_fromUtf8(const stl_string& target)
{
	stl_string ret;
	utf8_foreach(target, [&ret](unsigned int codepoint)
	{
		utf16_push_back(ret, codepoint);
	});
	return ret;
}