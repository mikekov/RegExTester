#pragma once
#include "wincore.h"

namespace mcr {

class CScrollBar : public CWnd
{
public:
	CScrollBar();
	virtual ~CScrollBar();

	int GetScrollPos() const;
	int SetScrollPos(int pos, bool redraw= true);
	void GetScrollRange(int* min_pos, int* max_pos) const;
	void SetScrollRange(int min_pos, int max_pos, bool redraw= true);
	void ShowScrollBar(bool show= true);

	BOOL EnableScrollBar(UINT arrow_flags= ESB_ENABLE_BOTH);

	BOOL SetScrollInfo(SCROLLINFO* scroll_info, bool redraw= true);
	BOOL GetScrollInfo(SCROLLINFO* scroll_info, UINT mask= SIF_ALL);
	int GetScrollLimit();

#if (_WIN32_WINNT >= 0x0501)
	BOOL GetScrollBarInfo(SCROLLBARINFO* scroll_info) const;
#endif	// _WIN32_WINNT >= 0x0501
};

} // namespace
