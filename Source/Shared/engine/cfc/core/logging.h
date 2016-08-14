#pragma once

#include <cfc/base.h>

CFC_NAMESPACE1(cfc)

#define CFC_LOG_ASSERT(log, x) if((x) == false) { log->Logf(cfc::logflags::SevAssert, "Assertion failed: %s", #x); CFC_BREAKPOINT; }
#define CFC_LOG_ASSERT_EXPLAIN(log, x, y) if((x) == false) { log->Logf(cfc::logflags::SevAssert, "Assertion failed: %s", y); CFC_BREAKPOINT; }

struct logflags
{
	enum Enumeration
	{
		None,
		ScpEngine = (1 << 0),
		ScpApplication = (1 << 1),
		SevProfiling = (1 << 2),
		SevDebug = (1 << 3),
		SevInfo = (1 << 4),
		SevWarning = (1 << 5),
		SevError = (1 << 6),
		SevCritical = (1 << 7),
		SevAssert = (1 << 8),
		SevCrash = (1 << 9),
	};
	const char* ToString(Enumeration flag) const;
};

class CFC_API logging : public cfc::object
{
public:
	u32 GetFilterMask() const { return m_filter; }
	void SetFilterMask(u32 allowedMask) { m_filter = allowedMask; }

	virtual void Log(const char* msg, u32 flags=logflags::ScpEngine | logflags::SevInfo);
	void Logf(u32 flags, const char* format, ...);
private:
	u32 m_filter;
};

CFC_END_NAMESPACE1(cfc)