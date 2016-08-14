#pragma once

#include <cfc/base.h>

CFC_NAMESPACE1(cfc)

class hashing;
class io;
class logging;
class random;
class timing;
class window;

struct context
{
	hashing* 	Hash		= nullptr;
	io* 		IO			= nullptr;
	logging* 	Log			= nullptr;
	random* 	Random		= nullptr;
	timing* 	Timing		= nullptr;
	window* 	Window		= nullptr;
};

CFC_END_NAMESPACE1(cfc)