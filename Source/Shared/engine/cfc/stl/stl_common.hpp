#pragma once

// export
#if defined(CFC_SHARED_LIBRARY_EXPORT)
# ifdef WIN32
#  define STL_API __declspec(dllexport)
# else
#  define STL_API
# endif
#elif defined(CFC_SHARED_LIBRARY_IMPORT)
# ifdef WIN32
#  define STL_API __declspec(dllimport)
# else
#  define STL_API
# endif
#else
# define STL_API
#endif

// base types
typedef float f32;
typedef double f64;
#if _WIN64 || __x86_64__ || __ppc64__
typedef unsigned long long u32_64;
#else
typedef unsigned long u32_64;
#endif
typedef u32_64 usize; // size_t

typedef unsigned long long u64;
typedef long long i64;
typedef unsigned int u32;
typedef int i32;
typedef short i16;
typedef unsigned short u16;
typedef char i8;
typedef unsigned char u8;
typedef u8 byte;

template<class T, class T2>
class stl_pair
{
public:
	stl_pair() {}
	stl_pair(const T& o1, const T2& o2) :first(o1), second(o2) {}
	T first;
	T2 second;
};

class stl_no_mutex
{
public:
	inline void lock() {}
	inline void unlock() {}
};

#ifdef _DEBUG
#define stl_assert(x)  { bool assert_cmp_value = (x); if (assert_cmp_value == false) { __debugbreak(); } }
#else
#define stl_assert(x) {(x);}
#endif

#ifndef stl_math_pi
# define stl_math_pi 3.1415926535897932384626433832795
# define stl_math_deg2rad(val) ((PI/180.0f)*val)
# define stl_math_rad2deg(val) ((val*180.0f)/PI)
# define stl_math_min(x,y) ((x)<(y)?(x):(y))
# define stl_math_max(x,y) ((x)>(y)?(x):(y))
# define stl_math_clamp(val, min, max) stl_math_min(stl_math_max(val, min), max)
# define stl_math_iroundupdiv(val, round) (((val) + (round) - 1) / (round))
# define stl_math_iroundup(val, round) (stl_math_iroundupdiv(val, round) * (round))
#endif