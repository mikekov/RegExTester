#pragma once
#include "wincore.h"

namespace mcr {


class CTreeCtrl : public CWnd
{
public:
	CTreeCtrl();

	BOOL Create(DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);
	BOOL CreateEx(DWORD ex_style, DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);

	//int GetItemCount() const;

	//int GetCurSel() const;
	//int SetCurSel(int item);

	bool GetItem(TVITEM* item) const;
	bool DeleteItem(HTREEITEM item);
	bool DeleteAllItems();
	HTREEITEM GetChildItem(HTREEITEM item) const;
	HTREEITEM GetNextSiblingItem(HTREEITEM item) const;
	DWORD_PTR GetItemData(HTREEITEM item) const;

	bool SetItem(TVITEM* item);

	bool SelectItem(HTREEITEM item);
	bool Expand(HTREEITEM item, UINT code);
};


} // namespace
