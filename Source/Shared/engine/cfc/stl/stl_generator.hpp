#pragma once

// generator/continuation for C++
// author: Andrew Fedoniouk @ terrainformatica.com
// idea borrowed from: "coroutines in C" Simon Tatham,
//   http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html

// modified by Sen
// -- note: this is very evil usage of a C switch statement's ability to not break on a case, and being able to straddle across scopes. 
//          it's a perfect way to make portable generators (coroutines) though that compile on basically any C compiler
// --       as long as you don't yield across a switch statement (!)

/* example:
STL_GEN_START(myGenerator)
{
	int i;
	STL_GEN_EMIT_BEGIN(int)
		for (i = 0; i < 100; i++)
			STL_GEN_YIELD(100-i);
	STL_GEN_EMIT_END()
};
int main()
{
	myGenerator gen;
	int v;
	while (gen(v))
		printf("%d\n", v);
}
*/

struct stl_generator
{
	int _line;
	stl_generator() :_line(0) {}
};

#define STL_GEN_START(NAME) struct NAME : public stl_generator

#define STL_GEN_EMIT_BEGIN(T) bool operator()(T& _rv) { \
	switch (_line) { \
	case 0:;

#define STL_GEN_EMIT_END() } _line = 0; return false; }

#define STL_GEN_YIELD(V)     \
		do { \
		\
		_line = __LINE__; \
		_rv = (V); return true; case __LINE__:; \
		} while (0)