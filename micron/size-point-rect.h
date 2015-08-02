#pragma once

namespace mcr {


class CSize : public SIZE
{
public:
	CSize()								{ cx = cy = 0; }
	CSize(int w, int h)					{ cx = w; cy = h; }
	CSize(SIZE size)					{ cx = size.cx; cy = size.cy; }
	CSize(POINT pt)						{ cx = pt.x; cx = pt.y; }

	void SetSize(int w, int h)			{ cx = w; cy = h; }

	bool operator == (SIZE sz) const	{ return cx == sz.cx && cy == sz.cy; }
	bool operator != (SIZE sz) const	{ return cx != sz.cx || cy != sz.cy; }

	CSize& operator += (SIZE s)			{ cx += s.cx; cy += s.cy; return *this; }
	CSize operator + (SIZE s) const		{ CSize tmp(*this); return tmp += s; }

	CSize& operator -= (SIZE s)			{ cx -= s.cx; cy -= s.cy; return *this; }
	CSize operator - (SIZE s) const		{ CSize tmp(*this); return tmp -= s; }
};


inline CSize CSizeFromDWord(DWORD dw)
{
	return CSize(static_cast<short>(LOWORD(dw)), static_cast<short>(HIWORD(dw)));
}


class CPoint : public POINT
{
public:
	CPoint()							{ x = 0; y = 0; }
	CPoint(int x, int y)				{ this->x = x; this->y = y; }
	CPoint(SIZE sz)						{ x = sz.cx; y = sz.cy; }
	CPoint(POINT pt)					{ x = pt.x ; y = pt.y; }

	void Offset(int dx, int dy)			{ x += dx; y += dy; }
	void SetPoint(int x, int y)			{ this->x = x; this->y = y; }

	BOOL operator == (POINT pt) const	{ return ((x == pt.x) && (y == pt.y)); }
	BOOL operator != (POINT pt) const	{ return ((x != pt.x) || (y != pt.y)); }

	CPoint& operator += (SIZE s)		{ x += s.cx; y += s.cy; return *this; }
	CPoint operator + (SIZE s) const	{ CPoint tmp(*this); return tmp += s; }

	CPoint& operator -= (SIZE s)		{ x -= s.cx; y -= s.cy; return *this; }
	CPoint operator - (SIZE s) const	{ CPoint tmp(*this); return tmp -= s; }

	CPoint& operator += (POINT p)		{ x += p.x; y += p.y; return *this; }
	CPoint operator + (POINT p) const	{ CPoint tmp(*this); return tmp += p; }

	CPoint& operator -= (POINT p)		{ x -= p.x; y -= p.y; return *this; }
	CPoint operator - (POINT p) const	{ CPoint tmp(*this); return tmp -= p; }

	CSize ToSize() const				{ return CSize(x, y); }
};


inline CPoint operator - (CPoint p)			{ return CPoint(-p.x, -p.y); }


inline CPoint PointFromLParam(LPARAM pt)
{
	return CPoint(static_cast<short>(LOWORD(pt)), static_cast<short>(HIWORD(pt)));
}



class CRect : public RECT
{
public:
	CRect()										{ left = top = right = bottom = 0; }
	CRect(int l, int t, int r, int b)			{ left = l; top = t; right = r; bottom = b; }
	CRect(const RECT& rc)						{ left = rc.left; top = rc.top; right = rc.right; bottom = rc.bottom; }
	CRect(POINT pt, SIZE sz)					{ SetRect(pt, sz); }
	CRect(POINT top_left, POINT bottom_right)	{ SetRect(top_left, bottom_right); }

	operator RECT* ()							{ return this; }
	operator const RECT* () const				{ return this; }

	bool operator == (const RECT& rc) const		{ return left == rc.left && right == rc.right && top == rc.top && bottom == rc.bottom; }
	bool operator != (const RECT& rc) const		{ return !(*this == rc); }

	bool EqualRect(const RECT* rect) const		{ return *this == *rect; }

	//CRect& operator = (const RECT& src)	default;		{ left = src.left; top = src.top; right = src.right; bottom = src.bottom; return *this; }
	void CopyRect(const RECT* rect)				{ *this = *rect; }

	int Height() const							{ return bottom - top; }
	int Width() const							{ return right - left; }
	CSize Size() const							{ return CSize(Width(), Height()); }

	void SetWidth(int width)					{ right = left + width; }
	void SetHeight(int height)					{ bottom = top + height; }

	void InflateRect(int l, int t, int r, int b)	{ left += l; right += r; top += t; bottom += b; }
	void InflateRect(int dx, int dy)			{ InflateRect(dx, dy, dx, dy); }
	void InflateRect(SIZE s)					{ InflateRect(s.cx, s.cy); }
	void InflateRect(const RECT* r)				{ InflateRect(r->left, r->top, r->right, r->bottom); }

	void DeflateRect(int l, int t, int r, int b)	{ left -= l; right -= r; top -= t; bottom -= b; }
	void DeflateRect(int dx, int dy)			{ DeflateRect(dx, dy); }
	void DeflateRect(SIZE s)					{ DeflateRect(s.cx, s.cy); }
	void DeflateRect(const RECT* r)				{ DeflateRect(r->left, r->top, r->right, r->bottom); }

	bool IsRectEmpty() const					{ return ::IsRectEmpty(this) != 0;}
	bool IsRectNull() const						{ return left == 0 && right == 0 && top == 0 && bottom == 0; }

	void OffsetRect(int dx, int dy)				{ left += dx; right += dx; top += dy; bottom += dy; }
	void OffsetRect(SIZE s)						{ OffsetRect(s.cx, s.cy); }
	void OffsetRect(POINT p)					{ OffsetRect(p.x, p.y); }

	bool PtInRect(POINT pt) const				{ return ::PtInRect(this, pt) != 0; }

	CRect& NormalizeRect()
	{
		if (left > right)	std::swap(left, right);
		if (top > bottom)	std::swap(top, bottom);
		return *this;
	}

	CRect& SetRect(int left, int top, int right, int bottom)
	{ this->left = left; this->top = top; this->right = right; this->bottom = bottom; return *this; }
	CRect& SetRect(POINT pt, SIZE sz)			{ left = pt.x, right = left + sz.cx; top = pt.y; bottom = top + sz.cy; return *this; }
	CRect& SetRect(POINT top_left, POINT bottom_right)
	{ left = top_left.x; top = top_left.y; right = bottom_right.x; bottom = bottom_right.y; return *this; }

	CRect& SetRectEmpty()						{ return SetRect(0, 0, 0, 0); }

	bool SubtractRect(const RECT* rect1, const RECT* rect2)
	{ return ::SubtractRect(this, rect1, rect2) != 0; }

	// returns true if union is non-empty
	bool UnionRect(const RECT* rect1, const RECT* rect2)
	{ return ::UnionRect(this, rect1, rect2) != 0; }

	// true if intersection is non-empty
	bool IntersectRect(const RECT* rect1, const RECT* rect2)
	{ return ::IntersectRect(this, rect1, rect2) != 0; }

	CRect& operator |= (const RECT& rect)		{ UnionRect(this, &rect); return *this; }
	CRect operator | (const RECT& rect) const	{ CRect tmp(*this); return tmp |= rect; }

	CRect& operator &= (const RECT& rect)		{ IntersectRect(this, &rect); return *this; }
	CRect operator & (const RECT& rect) const	{ CRect tmp(*this); return tmp &= rect; }

	CPoint CenterPoint() const					{ return CPoint((left + right) / 2, (top + bottom) / 2); }

	// no references returned
	CPoint GetTopLeft() const					{ return CPoint(left, top); }
	CPoint GetTopRight() const					{ return CPoint(right, top); }
	CPoint GetBottomLeft() const				{ return CPoint(left, bottom); }
	CPoint GetBottomRight() const				{ return CPoint(right, bottom); }

	// move corner without modifying dimension
	void SetTopLeft(CPoint c)					{ left = c.x; top = c.y; }
	void SetTopRight(CPoint c)					{ right = c.x; top = c.y; }
	void SetBottomLeft(CPoint c)				{ left = c.x; bottom = c.y; }
	void SetBottomRight(CPoint c)				{ right = c.x; bottom = c.y; }

	// MoveToX/Y
};


// scale rect
inline CRect RectMulDiv(const RECT& rect, int multiplier, int divisor)
{
	return CRect(
		::MulDiv(rect.left, multiplier, divisor),
		::MulDiv(rect.top, multiplier, divisor),
		::MulDiv(rect.right, multiplier, divisor),
		::MulDiv(rect.bottom, multiplier, divisor)
		);
}


} // namespace
