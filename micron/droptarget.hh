#pragma once
#include "cmdtarget.h"
//#include "wincore.h"

namespace mcr {


class COleDropTarget : public CCmdTarget
{
public:
	COleDropTarget();

//	bool Register(CWnd* wnd);

	virtual DROPEFFECT OnDragEnter(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point);
	virtual BOOL OnDrop(CWnd* wnd, COleDataObject* data_object, DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* wnd, COleDataObject* data_object, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	virtual void OnDragLeave(CWnd* wnd);
	virtual DROPEFFECT OnDragScroll(CWnd* wnd, DWORD key_state, CPoint point);

	virtual ~COleDropTarget();

};


} // namespace
