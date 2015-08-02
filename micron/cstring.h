#pragma once

namespace mcr {


class CString
{
public:
	CString();
	CString(const wchar_t* s);
	CString(const char* s);
	CString(const TCHAR* s, size_t length);

	void Format(const TCHAR* fmt, ...);
	size_t GetLength() const;
	bool IsEmpty() const			{ return str_.empty(); }
	bool LoadString(UINT id);
	void Empty()					{ clear(); }

	friend CString operator + (const TCHAR* s1, const CString& s2);

	CString& operator += (const TCHAR* str);
	CString operator + (const TCHAR* str);

	CString& operator += (const CString& str);
	CString operator + (const CString& str);

	operator const TCHAR* ();

	void clear()					{ str_.clear(); }
	bool empty() const				{ return str_.empty(); }
	void resize(size_t length)		{ str_.resize(length); }
	const TCHAR* data()				{ return str_.data(); }
	const TCHAR* c_str() const		{ return str_.c_str(); }
	size_t length() const			{ return str_.length(); }
	int int_length() const			{ return static_cast<int>(str_.length()); }
	CString& assign(const TCHAR* from, const TCHAR* to)	{ str_.assign(from, to); return *this; }
	CString& assign(const TCHAR* str, size_t count)		{ str_.assign(str, count); return *this; }
	const std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> >& std_str() const	{ return str_; }

private:
	std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > str_;
};


} // namespace
