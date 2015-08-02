#include "stdafx.h"
#include "micron.h"


BOOL CBitmap::CreateCompatibleBitmap(CDC* dc, int width, int height)
{
	HBITMAP hbmp= dc ? ::CreateCompatibleBitmap(dc->handle(), width, height) : 0;
	return Attach(hbmp);
}
