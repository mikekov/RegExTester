#pragma once
#include "wincore.h"

namespace mcr {


class CListCtrl : public CWnd
{
public:
	CListCtrl();

	BOOL Create(DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);
	BOOL CreateEx(DWORD ex_style, DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);

	//int GetItemCount() const;

	//int GetCurSel() const;
	//int SetCurSel(int item);

};


} // namespace
