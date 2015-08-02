#pragma once
#include "basetypes.h"
#include "gdiobj.h"

namespace mcr {

class CWnd;


class CDC
{
public:
	CDC();
	CDC(const CDC& rhs);

	virtual ~CDC();

	virtual BOOL TextOut(int x, int y, const TCHAR* string, int count);
	BOOL TextOut(int x, int y, const CString& str);

	CPen* SelectObject(CPen* pen);
	CBrush* SelectObject(CBrush* brush);
	virtual CFont* SelectObject(CFont* font);
	CBitmap* SelectObject(CBitmap* bitmap);
	virtual CGdiObject* SelectStockObject(int nIndex);

	CPoint MoveTo(int x, int y);
	CPoint MoveTo(POINT pt);
	CPoint GetCurrentPosition() const;

	bool LineTo(int x, int y);
	bool LineTo(POINT point);

	void FillSolidRect(int x, int y, int width, int height, COLORREF color);
	void FillSolidRect(const RECT* rect, COLORREF color);

	virtual COLORREF SetBkColor(COLORREF color);
	virtual COLORREF SetTextColor(COLORREF color);
	int SetBkMode(int bkMode);
	UINT SetTextAlign(UINT flags);

	virtual int DrawText(const TCHAR* string, size_t count, RECT* rect, UINT format);
	int DrawText(const CString& str, RECT* rect, UINT format);

	void Draw3dRect(const RECT* rect, COLORREF c1, COLORREF c2);
	void Draw3dRect(int x, int y, int w, int h, COLORREF c1, COLORREF c2);
	void DrawFocusRect(const RECT* rect);

	CSize GetTextExtent(const TCHAR* string, size_t count) const;
	CSize GetTextExtent(const CString& str) const;

	bool GetTextMetrics(TEXTMETRIC* metrics) const;

	bool BitBlt(int x, int y, int width, int height, CDC* src, int x_src, int y_src, DWORD rop);
	bool StretchBlt(int x, int y, int width, int height, CDC* src, int x_src, int y_src, int src_width, int src_height, DWORD rop);

	BOOL CreateCompatibleDC(CDC* dc);
	BOOL CreateIC(const TCHAR* driver_name, const TCHAR* device_name, const TCHAR* output, const void* init_data);

	bool Attach(HDC hdc);
	HDC Detach();
	static CDC* FromHandle(HDC dc);
	static CDC* FromHandlePermanent(HDC hdc);
	HDC handle() const		{ return handle_.get(); }

	void DeleteDC();

	int GetDeviceCaps(int index) const;

//	operator HDC() const { return handle(); }

	// Mapping Functions
	int GetMapMode() const;
	virtual int SetMapMode(int map_mode);
	// Viewport Origin
	CPoint GetViewportOrg() const;
	virtual CPoint SetViewportOrg(int x, int y);
			CPoint SetViewportOrg(POINT point);
	virtual CPoint OffsetViewportOrg(int width, int height);

	// Viewport Extent
	CSize GetViewportExt() const;
	virtual CSize SetViewportExt(int cx, int cy);
			CSize SetViewportExt(SIZE size);
//	virtual CSize ScaleViewportExt(int num, int denom, int num, int denom);

	// Window Origin
	CPoint GetWindowOrg() const;
	CPoint SetWindowOrg(int x, int y);
	CPoint SetWindowOrg(POINT point);
	CPoint OffsetWindowOrg(int width, int height);

	// Window extent
	CSize GetWindowExt() const;
	virtual CSize SetWindowExt(int cx, int cy);
			CSize SetWindowExt(SIZE size);
//	virtual CSize ScaleWindowExt(int num, int denom, int num, int denom);

	CWnd* GetWindow() const;

private:
	CDC& operator = (const CDC&);

	typedef handle_mixin<CDC, HDC> Handle;
	Handle handle_;
	struct details;
};



class CPaintDC : public CDC
{
public:
	explicit CPaintDC(CWnd* wnd);
	virtual ~CPaintDC();

	const PAINTSTRUCT& PaintStruct();

private:
	PAINTSTRUCT ps_;
	HWND wnd_;
};



class CClientDC : public CDC
{
public:
	explicit CClientDC(CWnd* wnd);
	virtual ~CClientDC();

private:
};


} // namespace mcr
