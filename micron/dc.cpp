#include "stdafx.h"
#include "micron.h"

struct CDC::details
{
	// grant hook helper access to the CDC internals
	struct CreateTemp;
};


struct CDC::details::CreateTemp
{
	CDC* operator () (HDC h)
	{
		CDC* dc= new CDC();
		dc->handle_.set_temp(h);
		return dc;
	}
};


CDC::CDC()
{
}


CDC::~CDC()
{
	if (handle_.has_ownership())
		::DeleteDC(Detach());
}


bool CDC::Attach(HDC hdc)					{ return handle_.attach(this, hdc); }
HDC CDC::Detach()							{ return handle_.detach(); }
CDC* CDC::FromHandlePermanent(HDC hdc)		{ return Handle::find_permanent(hdc); }
CDC* CDC::FromHandle(HDC hdc)				{ return Handle::find_or_create(hdc, details::CreateTemp()); }


#define BAIL_IF_NO_DC(...)		{ if (handle() == 0) { ASSERT(false); return __VA_ARGS__; } }


CWnd* CDC::GetWindow() const
{
	BAIL_IF_NO_DC(0);
	return CWnd::FromHandle(::WindowFromDC(handle()));
}


BOOL CDC::CreateCompatibleDC(CDC* dc)
{
	ASSERT(handle() == 0, L"empty CDC object expected");

	if (dc)
	{
		HDC hdc= ::CreateCompatibleDC(dc->handle());
		return Attach(hdc);
	}
	return false;
}


CPen* CDC::SelectObject(CPen* pen)
{
	BAIL_IF_NO_DC(0);
	HGDIOBJ hpen= ::SelectObject(handle(), pen->handle());
	return static_cast<CPen*>(CGdiObject::FromHandle(hpen));
}


CBrush* CDC::SelectObject(CBrush* brush)
{
	BAIL_IF_NO_DC(0);
	HGDIOBJ hbrush= ::SelectObject(handle(), brush->handle());
	return static_cast<CBrush*>(CGdiObject::FromHandle(hbrush));
}


CFont* CDC::SelectObject(CFont* font)
{
	BAIL_IF_NO_DC(0);
	HGDIOBJ hfont= ::SelectObject(handle(), font->handle());
	return static_cast<CFont*>(CGdiObject::FromHandle(hfont));
}


CBitmap* CDC::SelectObject(CBitmap* bitmap)
{
	BAIL_IF_NO_DC(0);
	HGDIOBJ hbmp= ::SelectObject(handle(), bitmap->handle());
	return static_cast<CBitmap*>(CGdiObject::FromHandle(hbmp));
}


CGdiObject* CDC::SelectStockObject(int index)
{
	BAIL_IF_NO_DC(0);
	HGDIOBJ object= ::GetStockObject(index);
	HGDIOBJ old_object= ::SelectObject(handle(), object);
	return CGdiObject::FromHandle(old_object);
}


BOOL CDC::TextOut(int x, int y, const TCHAR* string, int count)
{
	BAIL_IF_NO_DC(false);
	return ::TextOut(handle(), x, y, string, count);
}


BOOL CDC::TextOut(int x, int y, const CString& str)
{
	BAIL_IF_NO_DC(false);
	return ::TextOut(handle(), x, y, str.c_str(), static_cast<int>(str.length()));
}


CPoint CDC::MoveTo(int x, int y)
{
	CPoint old;
	BAIL_IF_NO_DC(old);
	::MoveToEx(handle(), x, y, &old);
	return old;
}


CPoint CDC::MoveTo(POINT pt)
{
	return MoveTo(pt.x, pt.y);
}


CPoint CDC::GetCurrentPosition() const
{
	CPoint point;
	BAIL_IF_NO_DC(point);
	::GetCurrentPositionEx(handle(), &point);
	return point;
}


void CDC::FillSolidRect(int x, int y, int width, int height, COLORREF color)
{
	BAIL_IF_NO_DC();
	::SetBkColor(handle(), color);
	CRect rect(x, y, x + width, y + height);
	::ExtTextOut(handle(), rect.left, rect.top, ETO_OPAQUE, rect, 0, 0, 0);
}

void CDC::FillSolidRect(const RECT* rect, COLORREF color)
{
	BAIL_IF_NO_DC();

	if (rect)
	{
		::SetBkColor(handle(), color);
		::ExtTextOut(handle(), rect->left, rect->top, ETO_OPAQUE, rect, 0, 0, 0);
	}
}


COLORREF CDC::SetBkColor(COLORREF color)
{
	BAIL_IF_NO_DC(0);
	return ::SetBkColor(handle(), color);
}


COLORREF CDC::SetTextColor(COLORREF color)
{
	BAIL_IF_NO_DC(0);
	return ::SetTextColor(handle(), color);
}


int CDC::SetBkMode(int bk_mode)
{
	BAIL_IF_NO_DC(0);
	return ::SetBkMode(handle(), bk_mode);
}


UINT CDC::SetTextAlign(UINT align)
{
	BAIL_IF_NO_DC(0);
	return ::SetTextAlign(handle(), align);
}


int CDC::DrawText(const TCHAR* string, size_t count, RECT* rect, UINT format)
{
	BAIL_IF_NO_DC(0);
	return ::DrawText(handle(), string, static_cast<int>(count), rect, format);
}


int CDC::DrawText(const CString& str, RECT* rect, UINT format)
{
	BAIL_IF_NO_DC(0);
	return ::DrawText(handle(), str.c_str(), static_cast<int>(str.length()), rect, format);
}


void CDC::Draw3dRect(int x, int y, int w, int h, COLORREF c1, COLORREF c2)
{
	BAIL_IF_NO_DC();
	FillSolidRect(x, y, w - 1, 1, c1);
	FillSolidRect(x, y, 1, h - 1, c1);
	FillSolidRect(x + w, y, -1, h, c2);
	FillSolidRect(x, y + h, w, -1, c2);
}


void CDC::Draw3dRect(const RECT* rect, COLORREF c1, COLORREF c2)
{
	Draw3dRect(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, c1, c2);
}


void CDC::DrawFocusRect(const RECT* rect)
{
	BAIL_IF_NO_DC();
	::DrawFocusRect(handle(), rect);
}


CSize CDC::GetTextExtent(const TCHAR* string, size_t count) const
{
	CSize size;
	BAIL_IF_NO_DC(size);
	::GetTextExtentPoint32(handle(), string, count, &size);
	return size;
}


CSize CDC::GetTextExtent(const CString& str) const
{
	return GetTextExtent(str.c_str(), str.length());
}


bool CDC::BitBlt(int x, int y, int width, int height, CDC* src, int x_src, int y_src, DWORD rop)
{
	BAIL_IF_NO_DC(false);
	return ::BitBlt(handle(), x, y, width, height, src ? src->handle() : 0, x_src, y_src, rop) != 0;
}


bool CDC::StretchBlt(int x, int y, int width, int height, CDC* src, int x_src, int y_src, int src_width, int src_height, DWORD rop)
{
	BAIL_IF_NO_DC(false);
	return ::StretchBlt(handle(), x, y, width, height, src ? src->handle() : 0, x_src, y_src, src_width, src_height, rop) != 0;
}


CPaintDC::CPaintDC(CWnd* wnd)
{
	wnd_ = wnd->handle();

	if (!Attach(::BeginPaint(wnd_, &ps_)))
		ThrowLastError(); //		AfxThrowResourceException();
}


CPaintDC::~CPaintDC()
{
	::EndPaint(wnd_, &ps_);
	Detach();
}


const PAINTSTRUCT& CPaintDC::PaintStruct()
{
	return ps_;
}


int CDC::SetMapMode(int map_mode)
{
	//TODO:
	return 0;
}

CPoint CDC::SetViewportOrg(int x, int y)
{
	return CPoint(0, 0);
}

CPoint CDC::OffsetViewportOrg(int width, int height)
{
	return CPoint(0, 0);
}

CSize CDC::SetViewportExt(int cx, int cy)
{
	return CSize(0, 0);
}

CSize CDC::SetWindowExt(int cx, int cy)
{
	return CSize(0, 0);
}
