#include "stdafx.h"
#include "micron.h"
#include "graphics.h"

namespace mcr {


CGraphics::CGraphics(const CWnd& wnd) : handle_(wnd.handle())
{}

CGraphics::CGraphics(const CDC& dc) : handle_(dc.handle())
{}


//CGraphics::CGraphics(const CGraphics& g) {}



} // namespace mcr
