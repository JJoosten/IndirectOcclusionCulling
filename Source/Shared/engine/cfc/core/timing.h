#pragma once

#include <cfc/base.h>

CFC_NAMESPACE1(cfc)

class CFC_API timing : public object
{
public:
	virtual double GetTimeSeconds();
	virtual u64 GetTimeNanoSeconds();
};



CFC_END_NAMESPACE1(cfc)