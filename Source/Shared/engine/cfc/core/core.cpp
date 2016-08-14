#include <cfc/base.h>

#include <cfc/stl/threading.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <atomic>
#include <stdio.h>
#include <math.h>

#include "context.h"
#include "io.h"
#include "profiling.h"
#include "timing.h"
#include "random.h"
#include "logging.h"
#include "window.h"
#include "hashing.h"

// ** Source Code
using namespace cfc;
#pragma region Helpers
#pragma endregion
#pragma region Context

static cfc::core::threading::atomic_int gObjectCounter;

cfc::object::object()
{
	gObjectCounter.fetch_add(1);
}

cfc::object::~object()
{
	if (gObjectCounter.fetch_add(-1) == 1)
		0; // printf("Engine cleanup successful. All objects have been accounted for.\n"); // all objects have been deleted..
}

int cfc::object::GetNumberOfObjectsAlive()
{
	return gObjectCounter.load();
}

#pragma endregion
#pragma region IO

cfc::iobuffer::~iobuffer()
{ 
	// destroy
	if (data)
		free(data);
	data = nullptr;
	size = cfc::invalid_index;
}

iobuffer::iobuffer(const iobuffer& o)
{
	// steal
	iobuffer& v = (iobuffer&)o;
	data = v.data;
	size = v.size;
	v.data = nullptr;
	v.size = cfc::invalid_index;
}


iobuffer& iobuffer::operator=(const iobuffer& o)
{
	// destroy
	if (data)
		free(data);
	data = nullptr;
	size = cfc::invalid_index;

	// steal
	iobuffer& v = (iobuffer&)o;
	data = v.data;
	size = v.size;
	v.data = nullptr;
	v.size = cfc::invalid_index;

	return *this;
}



#pragma endregion
#pragma region Randomizer

#include <stdlib.h>

unsigned int cfc::random::rand(void)
{
	return ::rand();
}
int cfc::random::randInt(int base, int range)
{
	return base + rand() % range;
}


#pragma endregion
#pragma region Logging
void cfc::logging::Log(const char* msg, unsigned int flags/*=Flags::ScpEngine | Flags::SevInfo*/)
{
	printf("%s", msg);
}

void cfc::logging::Logf(unsigned int flags, const char* format, ...)
{
	char dest[1024 * 16];
	va_list argptr;
	va_start(argptr, format);
#ifdef WIN32
	vsprintf_s(dest, (1024*16)-1, format, argptr);
#else
	vsprintf(dest, format, argptr);
#endif
	va_end(argptr);

	Log(dest, flags);
}

const char* cfc::logflags::ToString(Enumeration flag) const
{
	switch (flag)
	{
		case logflags::ScpApplication:			return "Application";
		case logflags::ScpEngine:				return "Engine";
		case logflags::SevAssert:				return "Assert";
		case logflags::SevCrash:				return "Crash";
		case logflags::SevCritical:				return "Critical";
		case logflags::SevDebug:				return "Debug";
		case logflags::SevInfo:					return "Info";
		case logflags::SevError:				return "Major";
		case logflags::SevWarning:				return "Minor";
		case logflags::SevProfiling:			return "Profiling";
		default:								return "Unknown";
	}
}

#pragma endregion
#pragma region Timing

#include <chrono>
static std::chrono::high_resolution_clock gClock;

double cfc::timing::GetTimeSeconds()
{
	i64 v = gClock.now().time_since_epoch().count();
	double dv = static_cast<double>(v);
	dv /= 1000000000.0;
	return dv;
}

u64 cfc::timing::GetTimeNanoSeconds()
{
	return gClock.now().time_since_epoch().count();
}

cfc::profiling::scopedtime::scopedtime(cfc::context& ctx, const char* scope) : m_ctx(ctx), m_scope(scope)
{
	m_time = m_ctx.Timing->GetTimeSeconds();
}

cfc::profiling::scopedtime::~scopedtime()
{
	double timeEnd = m_ctx.Timing->GetTimeSeconds();
	double timeElapsed = timeEnd - m_time;

	m_ctx.Log->Logf(cfc::logflags::ScpEngine | cfc::logflags::SevInfo, "Timescope \"%s\" elapsed: %f ms\n", m_scope, timeElapsed*1000.0);
}

#pragma endregion
#pragma region Hashing

u64 cfc::hashing::HashU64(const void* data, u32_64 length, u64 startHash) 
{ 
	return 0ULL; 
}
u32 cfc::hashing::HashU32(const void* data, u32_64 length, u32 startHash) 
{ 
	return 0U; 
}
#pragma endregion

// UNORDERED - ORDER THESE!
// ------|
//       v

