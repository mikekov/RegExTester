#include "stdafx.h"
#include "micron.h"
#include "button.h"

namespace mcr {


CCheckBox::CCheckBox()
{
	checked_ = false;
}


bool CCheckBox::GetChecked() const
{
	if (handle())
		checked_ = const_cast<CCheckBox*>(this)->SendMessage(BM_GETCHECK) == BST_CHECKED;

	return checked_;
}


void CCheckBox::SetChecked(bool checked)
{
	checked_ = checked;

	if (handle())
		SendMessage(BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED);
}


void CCheckBox::PreSubclassWindow()
{
	// init checkbox control with local variable
	SetChecked(checked_);
}


BOOL CCheckBox::OnChildNotify(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result)
{
	if (!sig_checked_.empty() && message == WM_COMMAND && HIWORD(wparam) == BN_CLICKED)
	{
		sig_checked_();
	}

	return CWnd::OnChildNotify(message, wparam, lparam, result);
}


mcr::slot_connection CCheckBox::OnChecked(mcr::Signal::fn callback)
{
	return sig_checked_.connect(callback);
}



CRadioButton::CRadioButton()
{
	selected_ = -1;
}


extern HWND find_first_radio_button(HWND button)
{
	if (button == 0)
		return 0;

	HWND first= button;

	for (HWND prev= button; ; )
	{
		DWORD style= ::GetWindowLong(prev, GWL_STYLE);

		if (style & WS_GROUP)	// head of current group reached?
			break;

		prev = ::GetWindow(prev, GW_HWNDPREV);

		if (prev == button || prev == 0)		// avoid cycles
			break;

		if ((::SendMessage(prev, WM_GETDLGCODE, 0, 0) & DLGC_RADIOBUTTON) && (style & BS_TYPEMASK) == BS_AUTORADIOBUTTON)
			first = prev;
	}

	return first;
}


void select_radio_button(HWND button, int selected)
{
	// find first radio button in a group
	HWND first= find_first_radio_button(button);

	if (first == 0)
		return;

	int button_index= 0;

	for (HWND hwnd= first; ; )
	{
		DWORD style= ::GetWindowLong(hwnd, GWL_STYLE);

		if ((::SendMessage(hwnd, WM_GETDLGCODE, 0, 0) & DLGC_RADIOBUTTON) && (style & BS_TYPEMASK) == BS_AUTORADIOBUTTON)
		{
			::SendMessage(hwnd, BM_SETCHECK, button_index == selected ? BST_CHECKED : BST_UNCHECKED, 0);
			button_index++;
		}

		hwnd = ::GetWindow(hwnd, GW_HWNDNEXT);
		if (hwnd == first || hwnd == 0)
			break;

		if (::GetWindowLong(hwnd, GWL_STYLE) & WS_GROUP)
			break;
	}
}


int find_selected(HWND button)
{
	// find first radio button in a group
	HWND first= find_first_radio_button(button);

	if (first == 0)
		return -1;

	int button_index= 0;

	for (HWND hwnd= first; ; )
	{
		DWORD style= ::GetWindowLong(hwnd, GWL_STYLE);

		if ((::SendMessage(hwnd, WM_GETDLGCODE, 0, 0) & DLGC_RADIOBUTTON) && (style & BS_TYPEMASK) == BS_AUTORADIOBUTTON)
		{
			if (::SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
				return button_index;

			button_index++;
		}

		hwnd = ::GetWindow(hwnd, GW_HWNDNEXT);
		if (hwnd == first || hwnd == 0)
			break;

		if (::GetWindowLong(hwnd, GWL_STYLE) & WS_GROUP)
			break;
	}

	return button_index;
}


void CRadioButton::PreSubclassWindow()
{
	// init checkbox control with local variable

	select_radio_button(handle(), selected_);
}


BOOL CRadioButton::OnChildNotify(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result)
{
//if (message == WM_COMMAND)
//{
//TCHAR m[140];
//wsprintf(m, L"r: %x %x %x %x\n", handle(), wparam, lparam, message);
//TRACE(m);
//}
//if (message == WM_NOTIFY)
//{
//	NMHDR* hdr= (NMHDR*)lparam;
//	int c= hdr->code;
//wsprintf(m, L"code: %d\n", c);
//TRACE(m);
////NM_SETFOCUS
//}

	if (!sig_selected_.empty() && message == WM_COMMAND && (HIWORD(wparam) == BN_CLICKED || HIWORD(wparam) == BN_UNPUSHED))
	{
		selected_ = find_selected(handle());
		sig_selected_();
	}

	return CWnd::OnChildNotify(message, wparam, lparam, result);
}

mcr::slot_connection CRadioButton::OnSelected(mcr::Signal::fn callback)
{
	return sig_selected_.connect(callback);
}


int CRadioButton::GetSelected() const
{
	if (handle())
		selected_ = find_selected(handle());

	return selected_;
}


void CRadioButton::SetSelected(int selected)
{
	selected_ = selected;

	if (handle())
		select_radio_button(handle(), selected_);
}

} // namespace
