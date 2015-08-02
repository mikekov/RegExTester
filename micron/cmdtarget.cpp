#include "stdafx.h"
#include "basedefs.h"
#include "cmdtarget.h"
#include "message_translator.h"

namespace mcr {


const mcr::message_map_struct* CCmdTarget::GetMessageMap() const
{
	return GetThisMessageMap();
}


const mcr::message_map_struct* CCmdTarget::GetThisMessageMap()
{
	// this the final entry in message map traversal through the hierarchy of derived classes
	static const mcr::message_map_container empty;
	static const mcr::message_map_struct msg_map= { 0, empty };
	return &msg_map;
}


CCmdTarget::CCmdTarget()
{}


bool CCmdTarget::OnCmdMsg(UINT id, int code, int msg, void* extra, LRESULT* result) //, AFX_CMDHANDLERINFO* pHandlerInfo);
{
	for (const message_map_struct* msg_map= GetMessageMap(); msg_map->get_base_map != 0; msg_map = msg_map->get_base_map())
		if (const message_map_entry* entry= msg_map->entries.find(msg, code, id))
			return DispatchCommandMessage(msg, id, *entry, extra, result);

	return false;
}


bool CCmdTarget::DispatchCommandMessage(int msg, UINT id, const mcr::message_map_entry& entry, void* extra, LRESULT* result)
{
	if (entry.handler.empty())
		return true;	// command 'handled'; that is ignored

	LRESULT res= 0;
	bool handled= false;

	switch (msg)
	{
	case WM_COMMAND:
	case WM_COMMAND | WM_REFLECT_FLAG:
		{
			if (entry.signature == mcr::FN_SIG_DEFAULT)
			{
				typedef msg_translator<WM_COMMAND, CCmdTarget>::CB Callback;
				const Callback& fn= boost::any_cast<const Callback&>(entry.handler);
				fn(this);
			}
			else if (entry.signature == mcr::FN_SIG_BOOL_UINT)
			{
				typedef msg_translator<WM_COMMAND, CCmdTarget, mcr::FN_SIG_BOOL_UINT>::CB Callback;
				const Callback& fn= boost::any_cast<const Callback&>(entry.handler);
				LRESULT r= fn(this, id);
			}
			else
			{
				ASSERT(false, L"unexpected fn signature");
				break;
			}
		}
		handled = true;
		break;

	case WM_NOTIFY:
	case WM_NOTIFY | WM_REFLECT_FLAG:
		{
			ASSERT(extra != 0, L"NMHDR pointer expected");
			NMHDR* hdr= static_cast<NMHDR*>(extra);

			if (entry.signature == mcr::FN_SIG_DEFAULT)
			{
				typedef msg_translator<WM_NOTIFY, CCmdTarget>::CB Callback;
				const Callback& fn= boost::any_cast<const Callback&>(entry.handler);
				fn(this, hdr, &res);
			}
			else if (entry.signature == mcr::FN_SIG_BOOL_UINT_NMHDR_RES)
			{
				typedef msg_translator<WM_NOTIFY, CCmdTarget, FN_SIG_BOOL_UINT_NMHDR_RES>::CB Callback;
				const Callback& fn= boost::any_cast<const Callback&>(entry.handler);
				res = fn(this, id, hdr, &res);
			}
			else
			{
				ASSERT(false, L"unexpected fn signature");
				break;
			}
		}
		handled = true;
		break;

	case WM_UPDATE_COMMAND_UI:
		{
			ASSERT(entry.signature == mcr::FN_SIG_DEFAULT, L"unexpected fn signature");
			ASSERT(extra != 0, L"CCmdUI pointer expected");
			CCmdUI* cmd_ui= static_cast<CCmdUI*>(extra);

			typedef msg_translator<WM_UPDATE_COMMAND_UI, CCmdTarget>::CB Callback;
			const Callback& fn= boost::any_cast<const Callback&>(entry.handler);
			fn(this, cmd_ui);
		}
		handled = true;
		break;

	default:
		ASSERT(false, L"unexpected message in the dispatch method");
		break;
	}

	if (result)
		*result = res;

	return handled;
}

} // namespace
