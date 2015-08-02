#include "stdafx.h"
#include "gdiobj.h"
#include "dc.h"

namespace mcr {


CGdiObject::CGdiObject() : m_hObject(0)
{}

CGdiObject::~CGdiObject()
{
	try
	{
		DeleteObject();
	}
	catch (...)
	{
		ASSERT(false, L"excption caught while deleting gdi object");
	}
}


struct CGdiObject::details
{
	// grant hook helper access to the CGdiObject internals
	struct CreateTemp;
};


struct CGdiObject::details::CreateTemp
{
	CGdiObject* operator () (HGDIOBJ h)
	{
		CGdiObject* dc= new CGdiObject();
		dc->handle_.set_temp(h);
		return dc;
	}
};


bool CGdiObject::Attach(HGDIOBJ h)						{ return handle_.attach(this, h); }
HGDIOBJ CGdiObject::Detach()							{ return handle_.detach(); }
CGdiObject* CGdiObject::FromHandlePermanent(HGDIOBJ h)	{ return Handle::find_permanent(h); }
CGdiObject* CGdiObject::FromHandle(HGDIOBJ h)			{ return Handle::find_or_create(h, details::CreateTemp()); }


bool CGdiObject::DeleteObject()
{
	if (!handle_.has_ownership())
		return false;

	return ::DeleteObject(Detach()) != 0;
}


} // namespace
