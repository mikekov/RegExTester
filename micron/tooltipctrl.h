#pragma once
#include "wincore.h"

namespace mcr {


class CToolTipCtrl : public CWnd
{
public:
	CToolTipCtrl();

	virtual BOOL Create(CWnd* parent_wnd, DWORD style= 0);

	void SetDelayTime(DWORD duration, int time);
	int SetMaxTipWidth(int width);

	int GetToolCount() const;
	void DelTool(CWnd* wnd, UINT_PTR tool_id= 0);
	bool AddTool(CWnd* wnd, UINT text_id, const RECT* tool_rect= 0, UINT_PTR tool_id= 0);
	bool AddTool(CWnd* wnd, const TCHAR* text= LPSTR_TEXTCALLBACK, const RECT* tool_rect= 0, UINT_PTR tool_id= 0);

};


} // namespace
