#pragma once

#include <cfc/base.h>

CFC_NAMESPACE2(cfc, profiling)

class CFC_API scopedtime
{
public:
	scopedtime(cfc::context& ctx, const char* scope);
	~scopedtime();
protected:
	cfc::context& m_ctx;
	const char* m_scope;
	double m_time;
};

CFC_END_NAMESPACE2(cfc, profiling)