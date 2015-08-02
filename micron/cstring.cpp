#include "stdafx.h"
#include "cstring.h"

namespace mcr {



size_t CString::GetLength() const
{
	return str_.length();
}


CString::operator const TCHAR* ()
{
	return str_.c_str();
}

CString::CString()
{}

CString::CString(const TCHAR* s) : str_(s)
{}


} // namespace