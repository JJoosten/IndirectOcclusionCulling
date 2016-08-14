#pragma once

CFC_NAMESPACE1(cfc)

class CFC_API random : public object
{
public:
	virtual u32 rand(void);
	f64 randDouble() { return rand() / (double)RAND_MAX; }
	i32 randInt(i32 base, i32 range);
};

CFC_END_NAMESPACE1(cfc)