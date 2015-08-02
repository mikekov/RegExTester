#pragma once

#include "stdafx.h"
#include "dialog.h"
#include "wincore.h"

namespace mcr {

// Simple dialog control resizer: create one calling CreateSimpleDlgCtrlResizer() and attach it to a dialog
// with a SetCtrlResizer call.
//
// Simple resizer takes a dialog window and a point inside this dialog. It will use the point to determine
// how to resize control windows. All windows to the left of the point.X line, do not move horizontally.
// Those to the right of the point.X line, will move when dialog gets resized. Finally those that cross
// point.X line, will grow/shrink as dialog resizes.
// Equivalent rules apply to the vertical resizing, with regard to the point.Y line.
//

class CSimpleDlgCtrlResizer : public CDlgResizer
{
public:
	CSimpleDlgCtrlResizer(CWnd& dialog, CPoint center);

	virtual void ResizeControls(CWnd& parent, int cx, int cy);

	virtual void SetWindowResizingFlags(HWND hwnd, ResizeFlag resize_flag, int half_flags= 0);

private:
	void BuildMap(CWnd* dialog, CPoint center);
	static BOOL CALLBACK EnumChildProcStatic(HWND wnd, LPARAM lParam);
	BOOL EnumChildProc(HWND wnd);
	void Resize(CRect rect);
	void Resize();

	struct Ctrl
	{
		Ctrl(HWND wnd) : wnd(wnd)
		{
			id = ::GetWindowLong(wnd, GWL_ID);
			WINDOWPLACEMENT wp;
			wp.length = sizeof wp;
			::GetWindowPlacement(wnd, &wp);
			rect = wp.rcNormalPosition;
			resize_flags = 0;
			half_flags = 0;
		}

		int id;
		int resize_flags;
		UINT half_flags;
		HWND wnd;
		CRect rect;
	};

	CWnd* dialog_;
	CRect dlg_rect_;
	std::vector<Ctrl> controls_;
	CSize shift_;
};


CSimpleDlgCtrlResizer::CSimpleDlgCtrlResizer(CWnd& dialog, CPoint center)
{
	BuildMap(&dialog, center);
}


void CSimpleDlgCtrlResizer::ResizeControls(CWnd& parent, int cx, int cy)
{
	if (dialog_->handle() != parent.handle())
		return;

	CRect rect(0, 0, cx, cy);
	Resize(rect);
}


void CSimpleDlgCtrlResizer::Resize()
{
	if (dialog_ == 0)
		return;

	CRect rect= dialog_->ClientRect();
	Resize(rect);
}


void CSimpleDlgCtrlResizer::Resize(CRect rect)
{
	if (dialog_ == 0)
		return;

	size_t count= controls_.size();

	int resize_count= 0;
	{
		// calc wnd to resize
		for (size_t i= 0; i < count; ++i)
			if (controls_[i].resize_flags != 0 || controls_[i].half_flags != 0)
				++resize_count;
	}

	CSize delta_size= rect.Size() - dlg_rect_.Size();

	HDWP wnd_pos_info= ::BeginDeferWindowPos(resize_count);

	for (size_t i= 0; i < count; ++i)
	{
		if ((controls_[i].resize_flags == 0 && controls_[i].half_flags == 0) || controls_[i].wnd == 0)
			continue;

		CRect rect= controls_[i].rect;
		UINT flags= SWP_NOZORDER | SWP_NOACTIVATE;
		CSize move_size= delta_size;
		CSize resize_size= delta_size;
		if (UINT half_flags= controls_[i].half_flags)
		{
			if (half_flags & HALF_MOVE_H)
				move_size.cx /= 2;
			if (half_flags & HALF_MOVE_V)
				move_size.cy /= 2;

			if (half_flags & HALF_RESIZE_H)
				resize_size.cx /= 2;
			if (half_flags & HALF_RESIZE_V)
				resize_size.cy /= 2;
		}

		WINDOWPLACEMENT wp;
		memset(&wp, 0, sizeof wp);
		wp.length = sizeof wp;
		if (::GetWindowPlacement(controls_[i].wnd, &wp) == 0)
		{
//			DWORD d= ::GetLastError();
			ASSERT(false);
			continue;
		}

		CSize shift(0, 0);
		if (controls_[i].half_flags & (SHIFT | SHIFT_RESIZES))
			shift = shift_;

		switch (controls_[i].resize_flags)
		{
		case MOVE_H:
			rect.OffsetRect(shift.cx + move_size.cx, shift.cy);
			flags |= SWP_NOSIZE;
			break;
		case MOVE_V:
			rect.OffsetRect(shift.cx, shift.cy + move_size.cy);
			flags |= SWP_NOSIZE;
			break;
		case MOVE:
			rect.OffsetRect(move_size + shift);
			flags |= SWP_NOSIZE;
			break;

		case MOVE_H_RESIZE_V:
			rect.OffsetRect(move_size.cx, 0);
			rect.bottom += resize_size.cy;
			if (controls_[i].half_flags & SHIFT_LEFT)
				rect.left += shift.cx;
			if (controls_[i].half_flags & SHIFT_RIGHT)
				rect.right += shift.cx;
			break;
		case MOVE_V_RESIZE_H:
			rect.OffsetRect(0, move_size.cy);
			rect.right += resize_size.cx;
			break;

		case MOVE_H_RESIZE_H:
			rect.OffsetRect(move_size.cx, 0);
			rect.right += resize_size.cx;
			break;
		case MOVE_H_RESIZE:
			rect.OffsetRect(move_size.cx, 0);
			rect.right += resize_size.cx;
			rect.bottom += resize_size.cy;
			break;
		case MOVE_V_RESIZE:
			rect.OffsetRect(0, move_size.cy);
			rect.right += resize_size.cx;
			rect.bottom += resize_size.cy;
			break;

		case RESIZE_H:
			rect.right += resize_size.cx;
			if (controls_[i].half_flags & SHIFT)
			{
				rect.left += shift.cx;
				rect.top += shift.cy;
				rect.bottom += shift.cy;
			}
			else if (controls_[i].half_flags & SHIFT_RESIZES)
			{
				rect.bottom += shift.cy;
			}
			else
			{
				rect.bottom = wp.rcNormalPosition.bottom;
				flags |= SWP_NOMOVE;
			}
			break;

		case RESIZE_V:
			rect.bottom += resize_size.cy;
			if (controls_[i].half_flags & SHIFT)
			{
				rect.left += shift.cx;
				rect.right += shift.cx;
				rect.top += shift.cy;
			}
			else if (controls_[i].half_flags & SHIFT_RESIZES)
			{
				rect.right += shift.cx;
			}
			else
			{
				rect.right = wp.rcNormalPosition.right;
				flags |= SWP_NOMOVE;
			}
			break;

		case RESIZE:
			rect.SetBottomRight(rect.GetBottomRight() + resize_size);
			if (controls_[i].half_flags & SHIFT_LEFT)
				rect.left += shift.cx;
			if (controls_[i].half_flags & SHIFT_RIGHT)
				rect.right += shift.cx;
//			rect.TopLeft() += shift;
			rect.top += shift.cy;
			if (shift == CSize(0, 0))
				flags |= SWP_NOMOVE;
			break;

		case NONE:
			if (shift != CSize(0, 0))
			{
				rect.OffsetRect(shift);
				flags &= ~SWP_NOMOVE;
			}
			break;
		}

		if (flags & SWP_NOMOVE)
			rect.SetTopLeft(CPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top));

		if (rect != wp.rcNormalPosition)		// placement changed?
		{
			if (wnd_pos_info)
				wnd_pos_info = ::DeferWindowPos(wnd_pos_info, controls_[i].wnd, 0, rect.left, rect.top, rect.Width(), rect.Height(), flags);
			else
				::SetWindowPos(controls_[i].wnd, 0, rect.left, rect.top, rect.Width(), rect.Height(), flags);

			::InvalidateRect(controls_[i].wnd, 0, true);
		}
	}

	if (wnd_pos_info)
		::EndDeferWindowPos(wnd_pos_info);
}


BOOL CSimpleDlgCtrlResizer::EnumChildProc(HWND wnd)
{
	if (wnd && ::GetParent(wnd) == dialog_->handle())
		controls_.push_back(Ctrl(wnd));
	return true;
}


BOOL CALLBACK CSimpleDlgCtrlResizer::EnumChildProcStatic(HWND wnd, LPARAM lParam)
{
	return reinterpret_cast<CSimpleDlgCtrlResizer*>(lParam)->EnumChildProc(wnd);
}


void CSimpleDlgCtrlResizer::BuildMap(CWnd* dialog, CPoint center)
{
	if (dialog->GetSafeHwnd() == 0)
		return;

	dialog_ = dialog;

	controls_.clear();
	controls_.reserve(32);

	::EnumChildWindows(dialog->handle(), EnumChildProcStatic, reinterpret_cast<LPARAM>(this));

	dlg_rect_ = dialog->ClientRect();

	const size_t count= controls_.size();
	for (size_t i= 0; i < count; ++i)
	{
		Ctrl& ctrl= controls_[i];
		const CRect& rect= ctrl.rect;

		if (rect.left < center.x && center.x < rect.right)
			ctrl.resize_flags |= RESIZE_H;
		if (rect.top < center.y && center.y < rect.bottom)
			ctrl.resize_flags |= RESIZE_V;

		if (rect.left > center.x)
			ctrl.resize_flags |= MOVE_H;
		if (rect.top > center.y)
			ctrl.resize_flags |= MOVE_V;
	}
}


void CSimpleDlgCtrlResizer::SetWindowResizingFlags(HWND hwnd, ResizeFlag resize_flag, int half_flags)
{
	bool found= false;
	const size_t count= controls_.size();
	for (size_t i= 0; i < count; ++i)
	{
		Ctrl& ctrl= controls_[i];
		if (ctrl.wnd == hwnd && ctrl.wnd != 0)
		{
			ctrl.resize_flags = resize_flag;
			ctrl.half_flags = half_flags;
			found = true;
			break;
		}
	}

	ASSERT(found);
}


boost::shared_ptr<CDlgResizer> CreateSimpleDlgCtrlResizer(CWnd& dialog, CPoint center)
{
	return boost::shared_ptr<CDlgResizer>(new CSimpleDlgCtrlResizer(dialog, center));
}


} // namespace
