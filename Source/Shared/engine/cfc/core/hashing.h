#pragma once

#include <cfc/base.h>

CFC_NAMESPACE1(cfc)

class CFC_API hashing : public object
{
public:
	virtual u64 HashU64(const void* data, u32_64 length, u64 startHash = 0xFFFFffffFFFFffffULL);
	virtual u32 HashU32(const void* data, u32_64 length, u32 startHash = 0xFFFFffffU);
};

CFC_END_NAMESPACE1(cfc)