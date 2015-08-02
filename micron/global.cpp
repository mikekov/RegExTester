#include "stdafx.h"
#include "micron.h"
//#include <boost/algorithm/string/find.hpp>
//#include <boost/algorithm/string/find_iterator.hpp>

namespace mcr {


HINSTANCE AppGetInstanceHandle()
{
	return GetApp()->GetInstanceHandle();
}


HINSTANCE AppGetResourceHandle()
{
	return GetApp()->GetInstanceHandle();
}


HINSTANCE AppFindResourceHandle(const TCHAR* name, const TCHAR* type)
{
	HINSTANCE instance= AppGetResourceHandle();
//	return ::FindResource(instance, name, type);
	return instance;
}


CWnd* AppGetMainWnd()
{
	return GetApp()->m_pMainWnd;
}


CWinApp* MicronGetApp()
{
	return GetApp();
}


bool MicronExtractSubString(CString& string, const TCHAR* full_string, int sub_string, TCHAR separator)
{
	string.clear();
	if (full_string == 0 || *full_string == 0 || sub_string < 0)
		return false;

	for (;;)
	{
		const TCHAR* from= full_string;
		const TCHAR* to= _tcschr(from, separator);

		if (sub_string == 0)
		{
			if (to == 0)
				to = from + _tcslen(from);

			string.assign(from, to);
			return true;
		}

		if (to == 0)
			break;

		--sub_string;
	}

	return false;

	/*
	typedef boost::split_iterator<const TCHAR*> string_split_iterator;

	for (string_split_iterator it= boost::make_split_iterator(
		full_string, boost::first_finder(separator, boost::is_equal()));
		it != string_split_iterator(); ++it)
	{
	//	cout << copy_range<std::string>(*It) << endl;
	}
	*/

}


} // namespace
