#pragma once
#include "cmdtarget.h"
//#include "wincore.h"

namespace mcr {

typedef DWORD DROPEFFECT;
class COleDataObject;


class COleDropTarget : public CCmdTarget
{
public:
	COleDropTarget();

//	bool Register(CWnd* wnd);

	virtual DROPEFFECT OnDragEnter(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point);
	virtual BOOL OnDrop(CWnd* wnd, COleDataObject* data_object, DROPEFFECT drop_effect, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* wnd, COleDataObject* data_object, DROPEFFECT drop_default, DROPEFFECT drop_list, CPoint point);
	virtual void OnDragLeave(CWnd* wnd);
	virtual DROPEFFECT OnDragScroll(CWnd* wnd, DWORD key_state, CPoint point);

	virtual ~COleDropTarget();

};


} // namespace
