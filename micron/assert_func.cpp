#include "stdafx.h"
#include "micron.h"

#include <assert.h>

namespace mcr {

#if _DEBUG
void assert_function(bool cond, const wchar_t* file_and_func, unsigned int line_no)
{
	if (!cond)
		_wassert(L"assertion failed", file_and_func, line_no);
}


void assert_function(bool cond, const wchar_t* msg, const wchar_t* file_and_func, unsigned int line_no)
{
	if (!cond)
		_wassert(msg, file_and_func, line_no);
}
#endif

} // namespace