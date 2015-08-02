#pragma once

#define _WIDEN2(x) L ## x
#define _WIDEN(x) _WIDEN2(x)

#define __WFILE__ _WIDEN(__FILE__)
#define __WFUNCTION__ _WIDEN(__FUNCTION__)

namespace mcr
{
	void assert_function(bool cond, const wchar_t* msg, const wchar_t* file_and_func, unsigned int line_no);
	void assert_function(bool cond, const wchar_t* file_and_func, unsigned int line_no);
}


#if _DEBUG
#  define ASSERT(cond,...)			mcr::assert_function(!!(cond), __VA_ARGS__, __WFILE__ L":" __WFUNCTION__, __LINE__)
#else
#  define ASSERT(cond,...)			{}
#endif


#if _DEBUG
#  define VERIFY(expr)				ASSERT((expr))
#else
#  define VERIFY(expr)				expr
#endif


template<class Arr>
inline int array_count(const Arr& array)	{ return sizeof array / sizeof array[0]; }
