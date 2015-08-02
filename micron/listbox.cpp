#include "stdafx.h"
#include "micron.h"


//namespace mcr {

struct CListBox::Impl
{
	CListBox::DrawItemSignal draw_item_;
	CListBox::MeasureItemSignal measure_item_;
};


CListBox::CListBox() : impl_(new Impl)
{}


CListBox::~CListBox()
{}


BOOL CListBox::OnChildNotify(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result)
{
	switch (message)
	{
	case WM_DRAWITEM:
		ASSERT(result == NULL);
		DrawItem(reinterpret_cast<DRAWITEMSTRUCT*>(lparam));
		break;

	case WM_MEASUREITEM:
		ASSERT(result == NULL);
		MeasureItem(reinterpret_cast<MEASUREITEMSTRUCT*>(lparam));
		break;

	case WM_COMPAREITEM:
		ASSERT(result != NULL);
		*result = CompareItem(reinterpret_cast<COMPAREITEMSTRUCT*>(lparam));
		break;

	case WM_DELETEITEM:
		ASSERT(result == NULL);
		DeleteItem(reinterpret_cast<DELETEITEMSTRUCT*>(lparam));
		break;

	case WM_VKEYTOITEM:
		*result = VKeyToItem(LOWORD(wparam), HIWORD(wparam));
		break;

	case WM_CHARTOITEM:
		*result = CharToItem(LOWORD(wparam), HIWORD(wparam));
		break;

	default:
		return CWnd::OnChildNotify(message, wparam, lparam, result);
	}

	return true;
}


int CListBox::SetCurSel(int select)
{
	ASSERT(::IsWindow(handle()));
	return static_cast<int>(::SendMessage(handle(), LB_SETCURSEL, select, 0));
}


int CListBox::GetCurSel() const
{
	ASSERT(::IsWindow(handle()));
	return static_cast<int>(::SendMessage(handle(), LB_GETCURSEL, 0, 0));
}


mcr::slot_connection CListBox::DrawItemEvent(const DrawItemSignal::fn& handler)
{
	return impl_->draw_item_.connect(handler);
}


mcr::slot_connection CListBox::MeasureItemEvent(const MeasureItemSignal::fn& handler)
{
	return impl_->measure_item_.connect(handler);
}


void CListBox::DrawItem(DRAWITEMSTRUCT* di)
{
	if (impl_->draw_item_.empty())
	{
		ASSERT(false, L"handle draw item");
	}
	else
		impl_->draw_item_(*this, di);
}


void CListBox::MeasureItem(MEASUREITEMSTRUCT* mi)
{
	if (impl_->measure_item_.empty())
	{
		ASSERT(false, L"handle measure item");
	}
	else
		impl_->measure_item_(*this, mi);
}


int CListBox::CompareItem(COMPAREITEMSTRUCT*)
{ ASSERT(false); return 0; }

void CListBox::DeleteItem(DELETEITEMSTRUCT*)
{ /* default to nothing */ }

int CListBox::VKeyToItem(UINT, UINT)
{ return int(Default()); }

int CListBox::CharToItem(UINT, UINT)
{ return int(Default()); }


//} // namespace
