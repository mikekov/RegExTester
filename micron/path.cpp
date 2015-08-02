#include "stdafx.h"
#include "micron.h"


CPath::CPath(const TCHAR* path) : str_(path)
{}


void CPath::Append(const TCHAR* str)
{
	str_ += L'\\';
	str_ += str;
}


CPath::operator const TCHAR* ()
{
	return str_.c_str();
}
