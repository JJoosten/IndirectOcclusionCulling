#pragma once
namespace stb
{
	typedef unsigned long long u64;
	u64 crc64_update(const void *buf, size_t len, u64 crc);
	u64 crc64(const void *buf, size_t len);
	u64 crc64_string(const char *t);
};