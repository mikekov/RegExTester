#pragma once
#include "wincore.h"

namespace mcr {


class CTabCtrl : public CWnd
{
public:
	CTabCtrl();

	virtual BOOL Create(DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);
	virtual BOOL CreateEx(DWORD ex_style, DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);

	int GetItemCount() const;

	int GetCurSel() const;
	int SetCurSel(int item);

};


} // namespace
