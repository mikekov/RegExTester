#include "stdafx.h"
#include "micron.h"

CFont::CFont()
{}


BOOL CFont::CreateFontIndirect(const LOGFONT* logFont)
{
	return Attach(::CreateFontIndirect(logFont));
}
