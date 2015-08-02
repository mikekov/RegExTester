#pragma once
#include "wincore.h"

namespace mcr {


class CFrameWnd : public CWnd
{
public:
	CFrameWnd();

	//BOOL Create(DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);
	//BOOL CreateEx(DWORD ex_style, DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);

	CWnd* CreateView(CCreateContext* context, UINT id= AFX_IDW_PANE_FIRST);

	CView* GetActiveView() const;
	void SetActiveView(CView* view, bool notify= true);

};


} // namespace
