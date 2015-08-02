#pragma once
#include "handle_map.h"
//#include "wincore.h"

namespace mcr {

	class CDC;


class CGdiObject
{
public:
	CGdiObject();
	virtual ~CGdiObject();

	bool DeleteObject();

	HGDIOBJ m_hObject;
	HGDIOBJ handle() const		{ return handle_.get(); }

	bool Attach(HGDIOBJ h);
	HGDIOBJ Detach();
	static CGdiObject* FromHandlePermanent(HGDIOBJ h);
	static CGdiObject* FromHandle(HGDIOBJ h);

private:
	typedef handle_mixin<CGdiObject, HGDIOBJ> Handle;
	Handle handle_;
	struct details;
};


class CPen : public CGdiObject
{
public:
	CPen(int pen_style, int width, COLORREF color);

};


class CBrush : public CGdiObject
{
public:

};


class CFont : public CGdiObject
{
public:
	CFont();

	BOOL CreateFontIndirect(const LOGFONT* logFont);

	BOOL CreateFont(int height, int width, int escapement, int orientation, int weight, BYTE italic, BYTE underline, BYTE strike_out, BYTE char_set, BYTE out_precision, BYTE clip_precision, BYTE quality, BYTE pitch_and_family, const TCHAR* facename);
};


class CBitmap : public CGdiObject
{
public:
	BOOL CreateCompatibleBitmap(mcr::CDC* dc, int width, int height);

};


} // namespace
