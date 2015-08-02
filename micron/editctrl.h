#pragma once
#include "wincore.h"

namespace mcr {


class CEdit : public CWnd
{
public:
	CEdit();

//	BOOL Create(DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);
//	BOOL CreateEx(DWORD ex_style, DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);

	//int GetItemCount() const;

	void ReplaceSel(const TCHAR* text, bool can_undo= false);

	void LimitText(int chars= 0);
	void SetSel(DWORD selection, bool no_scroll= false);
	void SetSel(int start_char, int end_char, bool no_scroll= false);

	bool SetReadOnly(bool read_only= true);

	bool GetModify() const;
	void SetModify(bool modified= true);
};


} // namespace
