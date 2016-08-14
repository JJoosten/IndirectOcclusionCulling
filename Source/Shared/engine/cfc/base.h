#pragma once

// ** CONFIGURATION HERE
#if defined(CFC_SHARED_LIBRARY_EXPORT)
# ifdef WIN32
#  define CFC_API __declspec(dllexport)
# else
#  define CFC_API
# endif
#elif defined(CFC_SHARED_LIBRARY_IMPORT)
# ifdef WIN32
#  define CFC_API __declspec(dllimport)
# else
#  define CFC_API
# endif
#else
# define CFC_API
#endif

#include "config.h"						

// ** END OF CONFIGURATION
#define CFC_LINE_SIG          __FILE__ ":"  CFC_STRINGIZE(__LINE__) " @ " __FUNCTION__
#define CFC_SWAP(a,b)         { auto __tmp(a); a=b; b=__tmp; } 

#undef CFC_BREAKPOINT
#ifdef _MSC_VER
#define CFC_BREAKPOINT { __debugbreak(); }
#else
#define CFC_BREAKPOINT  while(1) {}
#endif

// thread local
#ifdef _MSC_VER
#define CFC_THREAD_LOCAL_STORAGE __declspec(thread)
#elif __GNUC__
#define CFC_THREAD_LOCAL_STORAGE __thread
#else // not supported
#define CFC_THREAD_LOCAL_STORAGE
#endif

// disable optimization
#ifdef _MSC_VER
//#define CFC_DISABLE_OPTIMS __pragma(optimize("", off))
#endif

// check environment size
#if _WIN32 || _WIN64
#if _WIN64
#define CFC_ENVIRONMENT_64BIT
#else
#define CFC_ENVIRONMENT_32BIT
#endif
// Check GCC
#elif __GNUC__
#if __x86_64__ || __ppc64__
#define CFC_ENVIRONMENT_64BIT
#else
#define CFC_ENVIRONMENT_32BIT
#endif
#endif

#ifdef CFC_CONF_ENABLE_VLD
#ifdef WIN32
#include <vld.h>
#endif
#endif

#ifdef CFC_ENVIRONMENT_64BIT
#define CFC_SIZE_OF_PTR_IN_BYTES 8
#elif defined(CFC_ENVIRONMENT_32BIT)
#define CFC_SIZE_OF_PTR_IN_BYTES 4
#else
#error NOT IMPLEMENTED THIS ENVIRONMENT
#endif

#ifdef CFC_CONF_SAFETY
#define CFC_SAFELY(x) x
#else
#define CFC_SAFELY(x)
#endif

#define CFC_NAMESPACE1(x) namespace x {
#define CFC_NAMESPACE2(x,y) namespace x { namespace y {
#define CFC_NAMESPACE3(x,y,z) namespace x { namespace y { namespace z {
#define CFC_END_NAMESPACE1(x) };
#define CFC_END_NAMESPACE2(x,y) }; };
#define CFC_END_NAMESPACE3(x,y,z) }; }; };

#include "base.inl"
