#pragma once
#include "message_map.h"

namespace mcr {


class CCmdTarget
{
public:
	CCmdTarget();

	virtual bool OnCmdMsg(UINT id, int code, int msg, void* extra, LRESULT* result);//, AFX_CMDHANDLERINFO* pHandlerInfo);

	DECLARE_MESSAGE_MAP()

private:
	bool DispatchCommandMessage(int msg, UINT id, const mcr::message_map_entry& entry, void* extra, LRESULT* result);
};


}
