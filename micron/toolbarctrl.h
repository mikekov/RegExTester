#pragma once
#include "wincore.h"

namespace mcr {


class CToolBarCtrl : public CWnd
{
public:
	CToolBarCtrl();

	BOOL Create(DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);
	BOOL CreateEx(DWORD ex_style, DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);

	int GetItemCount() const;

	int GetCurSel() const;
	int SetCurSel(int item);

	bool EnableButton(int id, bool enable= true);
	bool CheckButton(int id, bool check= true);

	int SetHotItem(int hot);

	bool SetButtonInfo(int id, TBBUTTONINFO* tbi);

	bool GetItemRect(int index, RECT* rect) const;

	int GetButtonCount() const;

	bool AddButtons(int num_buttons, TBBUTTON* buttons);
	bool InsertButton(int index, TBBUTTON* button);
	bool DeleteButton(int index);

	void AutoSize();
};


} // namespace
