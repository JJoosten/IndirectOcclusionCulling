
#include <functional>

#include <cfc/platform/platform_win32.hpp>
#include <cfc/gpu/gpu_d3d12.h>
#include <cfc/stl/threading.h>
#include <thread>
#include <omp.h>
#include "cfc/math/math.h"
#include "cfc/gpu/gfx_d3d12.h"

extern void app_main(cfc::context* context);

void init(int argc, char** argv)
{
	cfc::context context;

	// * Initialize core components without context dependencies.
	cfc::timing timing;
	cfc::hashing hashing;
	cfc::logging log;
	cfc::random random;
	cfc::platform::win32::io io;
	context.Timing = &timing;
	context.Hash = &hashing;
	context.Log = &log;
	context.Random = &random;
	context.IO = &io;

	app_main(&context);
}

int main(int argc, char** argv)
{
	init(argc, argv);

	if (cfc::object::GetNumberOfObjectsAlive() > 0)
		CFC_BREAKPOINT; // memory leaks detected.
}


