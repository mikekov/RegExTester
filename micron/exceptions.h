#pragma once
#include "cstring.h"


namespace mcr {

class micron_exception : public std::exception
{
public:
	micron_exception(const char* msg) : std::exception(msg)
	{}
};


class file_exception : public micron_exception
{
public:
	file_exception() : micron_exception("file error")
	{
		cause_ = 0;
		os_error_ = 0;
	}

	int os_error_;
	uint32 cause_;
	CString file_name_;

	enum { badPath= 1 };
};


class last_err_exception : public mcr::micron_exception
{
	char buf[10];
public:
	last_err_exception() : mcr::micron_exception(_itoa(::GetLastError(), buf, 10))
	{}
};


inline void ThrowLastError()
{
	throw last_err_exception();
}

} // namespace
