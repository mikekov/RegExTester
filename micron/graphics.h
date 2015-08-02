#pragma once
#include "basetypes.h"
//#include "gdiobj.h"

namespace mcr {

class CWnd;
class CDC;


class CGraphics
{
public:
	CGraphics(const CWnd& wnd);
	CGraphics(const CDC& dc);

private:
	CGraphics& operator = (const CGraphics&);
	CGraphics(const CGraphics& rhs);
	CGraphics();

	graphics_details::Graphics handle_;
//	typedef handle_mixin<CDC, HDC> Handle;
//	Handle handle_;
//	struct details;
};


} // namespace mcr
