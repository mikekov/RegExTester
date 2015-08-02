#pragma once
#include "wincore.h"

namespace mcr {


class CView : public CWnd
{
public:
	CView();

//	BOOL Create(DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);
//	BOOL CreateEx(DWORD ex_style, DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);

	virtual void OnUpdate(CView* sender, LPARAM hint, void* hint_ptr);
	virtual void OnDraw(CDC* dc) = 0;

};


} // namespace
