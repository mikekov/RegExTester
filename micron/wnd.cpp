#include "stdafx.h"
#include "micron.h"
#include "handle_map.h"

namespace mcr {


BEGIN_MESSAGE_MAP(CWnd, CCmdTarget)
	ON_WM_NCDESTROY()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()


#define CATCH_ALL	catch(std::exception& ex)	{ ex; ASSERT(false); }


struct CWnd::details
{
	// grant hook helper access to the CWnd internals
	struct WndCreationHook;
	struct CreateTemp;
};


CWnd::CWnd()
{
	clear_out_all_vars();
}


CWnd::~CWnd()
{
	try
	{
		if (handle_.has_ownership())
			DestroyWindow();
	}
	catch (...)
	{
		ASSERT(false, L"excption caught while deleting window object");
	}
}


CWnd::CWnd(HWND hwnd, CreationParameter p)
{
	clear_out_all_vars();

	if (hwnd == 0)
	{	ASSERT(false, L"window expected"); }
	else if (p == CreatePermanent)
		Attach(hwnd);
	else if (p == CreateTemporary)
		handle_.set_temp(hwnd);
	else
	{ ASSERT(false, L"wrong window create param"); }
}


void CWnd::clear_out_all_vars()
{
	original_wnd_proc_ = 0;
	continue_modal_ = false;
	modal_result_ = 0;
	owner_wnd_ = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////

////// TODO: migrate to thread-specific globals ////////////
__declspec(thread) MSG global_cur_msg;
__declspec(thread) MSG global_last_sent_msg;
__declspec(thread) HWND global_disable_notifications= 0;
////////////////////////////////////////////////////////////


MSG& AppGetCurrentMessage()
{
	return global_cur_msg;
}

MSG& AppGetLastSentMessage()
{
	return global_last_sent_msg;
}

const MSG* CWnd::GetCurrentMessage()
{
	return &AppGetLastSentMessage();
}


LRESULT CWnd::CallWndProc(CWnd* wnd, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	MSG& m= AppGetLastSentMessage();
	MSG copy= m;

	// current message
	m.message = msg;
	m.wParam = wparam;
	m.lParam = lparam;
	m.hwnd = hwnd;

	LRESULT res= 0;

	try
	{
		res = wnd->WindowProc(msg, wparam, lparam);
	}
	CATCH_ALL

	m = copy;

	return res;
}


LRESULT CALLBACK CWnd::StdWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	CWnd* wnd= CWnd::FromHandlePermanent(hwnd);
	if (wnd != 0 && wnd->handle() == hwnd)
		return CWnd::CallWndProc(wnd, hwnd, msg, wparam, lparam);
	else
		return ::DefWindowProc(hwnd, msg, wparam, lparam);
}


LRESULT CWnd::WindowProc(UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result= 0;

	if (!OnWndMsg(message, wparam, lparam, &result))
		result = DefWindowProc(message, wparam, lparam);
//int i= ::GetLastError();
	return result;
}


BOOL CWnd::OnWndMsg(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result)
{
#if 0
TCHAR buf[200];
wsprintf(buf, L"msg: %04x  wnd: %p\n", message, handle());
::OutputDebugString(buf);
#endif

	LRESULT temp= 0;
	if (result == 0)
		result = &temp;

	switch (message)	// some messages get special treatment
	{
	case WM_COMMAND:
		if (OnCommand(wparam, lparam))
		{
			*result = 1;
			return true;
		}
		return false;

	case WM_NOTIFY:
		{
			NMHDR* hdr= reinterpret_cast<NMHDR*>(lparam);
			if (hdr->hwndFrom != 0 && OnNotify(wparam, lparam, result))
				return true;
			return false;
		}

	default:
		break;
	}

	// dispatch message

	for (const message_map_struct* msg_map= GetMessageMap(); msg_map->get_base_map != 0; msg_map = msg_map->get_base_map())
	{
		if (const message_map_entry* entry= msg_map->entries.find(message, 0u))
		{
			mcr::Msg m(message, wparam, lparam);

			if (DispatchWndMessage(m, *entry))
			{
				*result = m.result;
				return true;
			}

			return false;
		}
	}

	return false;
}


bool CWnd::DispatchWndMessage(mcr::Msg& msg, const mcr::message_map_entry& entry)
{
	if (entry.handler.empty())
		return false;

#define MSG_TRANSLATOR(WM)		msg_translator<(WM), CCmdTarget>
#define CALL_MSG_HANDLER(WM)	case (WM): \
		MSG_TRANSLATOR(WM)::call(this, msg, boost::any_cast<const MSG_TRANSLATOR(WM)::CB&>(entry.handler)); \
		break

	switch (msg.msg)
	{
	CALL_MSG_HANDLER(WM_SIZE);
	CALL_MSG_HANDLER(WM_PAINT);
	CALL_MSG_HANDLER(WM_ERASEBKGND);
	CALL_MSG_HANDLER(WM_INITDIALOG);
	CALL_MSG_HANDLER(WM_MEASUREITEM);
	CALL_MSG_HANDLER(WM_DRAWITEM);
	CALL_MSG_HANDLER(WM_MOUSEMOVE);
	CALL_MSG_HANDLER(WM_LBUTTONUP);
	CALL_MSG_HANDLER(WM_LBUTTONDOWN);
	CALL_MSG_HANDLER(WM_TIMER);
	CALL_MSG_HANDLER(WM_GETMINMAXINFO);
	CALL_MSG_HANDLER(WM_CLOSE);
	CALL_MSG_HANDLER(WM_NCDESTROY);

	default:
		if (entry.signature == FN_SIG_GENERIC_MESSAGE_CALLBACK)
		{
			const GenericMessageCallback& cb= boost::any_cast<const GenericMessageCallback&>(entry.handler);
			msg.result = cb(this, msg.wparam, msg.lparam);
			break;
		}
		else
		{
			ASSERT(false, L"dispatch is not implemented");
			return false;
		}
	}

#undef CALL_MSG_HANDLER
#undef MSG_TRANSLATOR

	return true;
}



// window initial subclassing helper relying on CBT hook to intercept window creation moment;
// modifies wnd proc allowing for handling of all window messages since window's inception, including WM_CREATE/WM_NCCREATE
//
struct CWnd::details::WndCreationHook
{
	WndCreationHook(CWnd* wnd) : unhook_(false)
	{
		if (old_hook_proc_ && wnd_)
		{
			ASSERT(false, L"cannot install new hook proc while previous one is still pending");
			return;
		}

		wnd_ = wnd;

		if (old_hook_proc_ == 0)	// reuse hook proc if it is already installed
		{
			old_hook_proc_ = ::SetWindowsHookEx(WH_CBT, &WndCreationHook::CbtFilterHook, NULL, ::GetCurrentThreadId());
			if (old_hook_proc_ == 0)
				ThrowLastError();
		}

		unhook_ = true;
	}

	~WndCreationHook()
	{
		if (unhook_ && old_hook_proc_ != 0)
		{
			::UnhookWindowsHookEx(old_hook_proc_);
			old_hook_proc_ = 0;
		}
	}

private:
	bool unhook_;
	static __declspec(thread) HHOOK old_hook_proc_;
	static __declspec(thread) CWnd* wnd_;

	static LRESULT CALLBACK CbtFilterHook(int code, WPARAM wparam, LPARAM lparam)
	{
		if (code == HCBT_CREATEWND && wnd_)
		{
			CWnd* w= wnd_;

			wnd_ = 0;	// reset this static var; do not init this window again

			HWND hwnd= reinterpret_cast<HWND>(wparam);

			ASSERT(CWnd::FromHandlePermanent(hwnd) == 0);
			ASSERT(w->handle() == 0);

			w->Attach(hwnd);
			w->PreSubclassWindow();

			w->SetInternalWndProc();
		}

		return CallNextHookEx(old_hook_proc_, code, wparam, lparam);
	}

};


__declspec(thread) HHOOK CWnd::details::WndCreationHook::old_hook_proc_= 0;
__declspec(thread) CWnd* CWnd::details::WndCreationHook::wnd_= 0;


void CWnd::SetInternalWndProc()
{
	WNDPROC std_proc= &CWnd::StdWndProc;
	WNDPROC old_proc= reinterpret_cast<WNDPROC>(::SetWindowLongPtr(handle(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(std_proc)));
	if (old_proc != std_proc)
		original_wnd_proc_ = old_proc;
}


void CWnd::PreSubclassWindow()
{}


BOOL CWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	return true;
}


void CWnd::PostNcDestroy()
{}


BOOL CWnd::Create(const TCHAR* className, const TCHAR* windowName, DWORD style, const RECT& rect, CWnd* parentWnd, UINT id, CCreateContext* context)
{
	return CreateEx(0, className, windowName, style | WS_CHILD, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, parentWnd->GetSafeHwnd(), reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)), context) != 0;
}


// advanced creation (allows access to extended styles)
BOOL CWnd::CreateEx(DWORD exStyle, const TCHAR* className, const TCHAR* windowName, DWORD style, int x, int y, int width, int height, HWND parent, HMENU id_or_menu, void* param)
{
	if (handle())
	{
		ASSERT(false, L"window already attached");
		return false;
	}

	CREATESTRUCT cs;
	cs.dwExStyle = exStyle;
	cs.lpszClass = className;
	cs.lpszName = windowName;
	cs.style = style;
	cs.x = x;
	cs.y = y;
	cs.cx = width;
	cs.cy = height;
	cs.hwndParent = parent;
	cs.hMenu = id_or_menu;
	cs.hInstance = AppGetInstanceHandle();
	cs.lpCreateParams = param;

	// customize creation params
	if (!PreCreateWindow(cs))
	{
		PostNcDestroy();
		return false;
	}

	{
		CWnd::details::WndCreationHook hook(this);

		HWND h= ::CreateWindowEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);

		ASSERT(h == handle(), L"hook wnd init failed");
	}

	return handle() != 0;
}


BOOL CWnd::CreateEx(DWORD exStyle, const TCHAR* className, const TCHAR* windowName, DWORD style, const RECT& rect, CWnd* parentWnd, UINT id, void* param)
{
	int x = rect.left;
	int y = rect.top;
	int cx = rect.right - rect.left;
	int cy = rect.bottom - rect.top;

	return CreateEx(exStyle, className, windowName, style, x, y, cx, cy, parentWnd->GetSafeHwnd(), reinterpret_cast<HMENU>(id), param);
}


static INT_PTR CALLBACK DialogProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return 0;
}


bool CWnd::CreateDlgIndirect(const DLGTEMPLATE* dialog_template, CWnd* parent_wnd, HINSTANCE instance)
{
	ASSERT(dialog_template != NULL, L"dlg template expected");
	//if (parent_wnd != NULL)
	//	ASSERT_VALID(parent_wnd);

	if (instance == 0)
		instance = AppGetResourceHandle();

	//HGLOBAL hTemplate = NULL;

	HWND hWnd = NULL;
#ifdef _DEBUG
	DWORD dwError = 0;
#endif

//	TRY
	{
//		VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG));
//		AfxDeferRegisterClass(AFX_WNDCOMMCTLSNEW_REG);

//#ifdef _UNICODE
//		AfxInitNetworkAddressControl();
//#endif
/*
		// If no font specified, set the system font.
		CString strFace;
		WORD wSize = 0;
		BOOL bSetSysFont = !CDialogTemplate::GetFont(dialog_template, strFace,
			wSize);

		if (bSetSysFont)
		{
			CDialogTemplate dlgTemp(dialog_template);
			dlgTemp.SetSystemFont(wSize);
			hTemplate = dlgTemp.Detach();
		}

		if (hTemplate != NULL)
			dialog_template = (DLGTEMPLATE*)GlobalLock(hTemplate);
*/
		// setup for modal loop and creation
//		dlg_result_ = -1;
//		m_nFlags |= WF_CONTINUEMODAL;

		// create modeless dialog
		{
			CWnd::details::WndCreationHook hook(this);

			hWnd = ::CreateDialogIndirect(instance, dialog_template, parent_wnd->GetSafeHwnd(), &DialogProc);

#ifdef _DEBUG
		dwError = ::GetLastError();
#endif
		}
	}
	//CATCH_ALL(e)
	//{
	//	DELETE_EXCEPTION(e);
	//	dlg_result_ = -1;
	//}
	//END_CATCH_ALL

	/* This is a bit tricky.  At this point, 1 of 3 things has happened:
 	 * 1) ::CreateDialogIndirect() created successfully and hWnd != NULL.
 	 * 2) ::CreateDialogIndirect() did create a window and then send the appropiate 
 	 *    creation messages (ie. WM_CREATE).  However, the user handled WM_CREATE and 
 	 *    returned -1.  This causes windows to send WM_DESTROY and WM_NCDESTROY to the
 	 *    newly created window.  Since WM_NCDESTROY has been sent, the destructor of this
 	 *    CWnd object has been called.  And ::CreateDialogIndirect() returns NULL.
 	 * 3) ::CreateDialogIndirect() did NOT create the window (ie. due to error in template)
 	 *    and returns NULL. 
 	 *
 	 * (Note: In 3, this object is still valid; whereas in 2, this object has been deleted).
 	 *
 	 * Adding to the complexity, this function needs to do 2 memory clean up (call 
 	 * pOccManager->PostCreateDialog() and delete occDialogInfo) if the destructor of 
 	 * this object hasn't been called.  If the destructor has been called, the clean up is done
 	 * in the destructor.
 	 *
 	 * We can use the return valid of AfxUnhookWindowCreate() to differentiate between 2 and 3.
 	 *  - If AfxUnhookWindowCreate() returns true and hWnd==NULL, this means that (2) has happened
 	 *    and we don't have to clean up anything. (Cleanup should be done in the destructor).
 	 *  - If AfxUnhookWindowCreate() returns false and hWnd== NULL, this means that (3) has happened
 	 *    and we need to call PostNcDestroy().
 	 *
 	 * Note: hWnd != NULL implies that AfxUnhookWindowCreate() return TRUE.
         *
         * Note2: From this point on, don't access any member variables without checking hWnd.  If 
         *        hWnd == NULL, the object has been destroyed already.
 	 */

//	if (!AfxUnhookWindowCreate())
//		PostNcDestroy();        // cleanup if Create fails too soon

	// handle EndDialog calls during OnInitDialog

	//if (hWnd != NULL && !(m_nFlags & WF_CONTINUEMODAL))
	//{
	//	::DestroyWindow(hWnd);
	//	hWnd = NULL;
	//}

	//if (hTemplate != NULL)
	//{
	//	GlobalUnlock(hTemplate);
	//	GlobalFree(hTemplate);
	//}

	if (hWnd == NULL)
		return false;

	ASSERT(handle() == hWnd);
	return true;
}


bool CWnd::SubclassWindow(HWND hwnd)
{
	if (!Attach(hwnd))
		return false;

	// allow any other subclassing to occur
	PreSubclassWindow();

	SetInternalWndProc();
/*
	// now hook into the AFX WndProc
	WNDPROC* lplpfn = GetSuperWndProcAddr();
	WNDPROC oldWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC,
		(INT_PTR)AfxGetAfxWndProc());
	ASSERT(oldWndProc != AfxGetAfxWndProc());

	if (*lplpfn == NULL)
		*lplpfn = oldWndProc;   // the first control of that type created
*/
//#ifdef _DEBUG
//	else if (*lplpfn != oldWndProc)
//	{
//		TRACE(traceAppMsg, 0, "Error: Trying to use SubclassWindow with incorrect CWnd\n");
//		TRACE(traceAppMsg, 0, "\tderived class.\n");
//		TRACE(traceAppMsg, 0, "\thWnd = $%08X (nIDC=$%08X) is not a %hs.\n", (UINT)(UINT_PTR)hWnd,
//			_AfxGetDlgCtrlID(hWnd), GetRuntimeClass()->m_lpszClassName);
//		ASSERT(FALSE);
//		// undo the subclassing if continuing after assert
//	  ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (INT_PTR)oldWndProc);
//	}
//#endif

	return true;
}


HWND CWnd::GetSafeHwnd() const
{
	return this == 0 ? 0 : handle();
}


struct CWnd::details::CreateTemp
{
	CWnd* operator () (HWND h)
	{
		CWnd* w= new CWnd();
		w->handle_.set_temp(h);
		return w;
	}
};


bool CWnd::Attach(HWND hwnd)				{ return handle_.attach(this, hwnd); }
HWND CWnd::Detach()							{ return handle_.detach(); }
CWnd* CWnd::FromHandlePermanent(HWND hwnd)	{ return Handle::find_permanent(hwnd); }
CWnd* CWnd::FromHandle(HWND hwnd)			{ return Handle::find_or_create(hwnd, details::CreateTemp()); }


BOOL CWnd::ContinueModal()
{
	return continue_modal_;
}


void CWnd::SetContinueModal()
{
	continue_modal_ = true;
}


void CWnd::EndModalLoop(int result)
{
	ASSERT(::IsWindow(handle()));

	modal_result_ = result;

	if (continue_modal_)
	{
		continue_modal_ = false;
		PostMessage(WM_NULL);	// wake up call
	}
}


struct BlockNotifications
{
	BlockNotifications(HWND hwnd)
	{
		ASSERT(global_disable_notifications == 0, L"notification are already being blocked");
		global_disable_notifications = hwnd;
	}

	~BlockNotifications()
	{
		global_disable_notifications = 0;
	}
};


bool CWnd::UpdateData(bool save_and_validate)
{
	ASSERT(::IsWindow(handle()));

	CDataExchange dx(this, save_and_validate);

	// block notifications
	BlockNotifications wnd(handle());

	try
	{
		DoDataExchange(&dx);

		return true;
	}
	catch (mcr::micron_exception& )
	{
		//TODO: report it
	}

	return false;
/*
	// prevent control notifications from being dispatched during UpdateData
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	HWND hWndOldLockout = pThreadState->m_hLockoutNotifyWindow;
	ASSERT(hWndOldLockout != m_hWnd);   // must not recurse
	pThreadState->m_hLockoutNotifyWindow = m_hWnd;

	BOOL bOK = FALSE;       // assume failure
	TRY
	{
		DoDataExchange(&dx);
		bOK = TRUE;         // it worked
	}
	CATCH(CUserException, e)
	{
		// validation failed - user already alerted, fall through
		ASSERT(!bOK);											
		// Note: DELETE_EXCEPTION_(e) not required
	}
	AND_CATCH_ALL(e)
	{
		// validation failed due to OOM or other resource failure
		e->ReportError(MB_ICONEXCLAMATION, AFX_IDP_INTERNAL_FAILURE);
		ASSERT(!bOK);
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	pThreadState->m_hLockoutNotifyWindow = hWndOldLockout;
	return bOK;
*/
}


CDataExchange::CDataExchange(CWnd* dlg_wnd, bool save_and_validate)
{
//	ASSERT_VALID(pDlgWnd);
	save_and_validate_ = save_and_validate;
	dlg_ = dlg_wnd;
//	m_idLastControl = 0;
}


HWND CDataExchange::PrepareCtrl(int id)
{
	return 0;
}


HWND CDataExchange::PrepareEditCtrl(int id)
{
	return 0;
}


void CDataExchange::Fail()
{}


void CWnd::DoDataExchange(CDataExchange* dx)
{}


LRESULT CWnd::Default()
{
	const MSG* msg= GetCurrentMessage();
	return DefWindowProc(msg->message, msg->wParam, msg->lParam);
}


LRESULT CWnd::DefWindowProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (original_wnd_proc_)
		return ::CallWindowProc(original_wnd_proc_, handle(), msg, wparam, lparam);

	return ::DefWindowProc(handle(), msg, wparam, lparam);
}


bool CWnd::WalkPreTranslateTree(HWND last_wnd, MSG& msg)
{
	ASSERT(last_wnd == 0 || ::IsWindow(last_wnd));

	// walk from child to parents; for all permanent CWnd along that way check to see if
	// they want to consume this message before it reaches its final destination
	for (HWND hwnd= msg.hwnd; hwnd; hwnd = ::GetParent(hwnd))
	{
		if (CWnd* wnd= CWnd::FromHandlePermanent(hwnd))
			if (wnd->PreTranslateMessage(&msg))
				return true;

		if (hwnd == last_wnd)
			break;
	}

	return false;
}


static bool AppPreTranslateMessage(MSG& msg)
{
	//TODO: dispatch to the current thread pre translate

	CWnd* main_wnd= mcr::AppGetMainWnd();

	if (CWnd::WalkPreTranslateTree(main_wnd->GetSafeHwnd(), msg))
		return true;

	//  check window's accelerator table
	if (main_wnd)
	{
		CWnd* wnd = CWnd::FromHandle(msg.hwnd);
		if (wnd->GetTopLevelParent() != main_wnd)
			return main_wnd->PreTranslateMessage(&msg) != 0;
	}

	return false;
}


static bool PumpMessage()
{
	MSG& msg= AppGetCurrentMessage();

	if (!::GetMessage(&msg, 0, 0, 0))
		return false;

	if (msg.message != WM_KICKIDLE && !AppPreTranslateMessage(msg))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return true;
}


static bool IsIdleMessage(const MSG& msg)
{
	// paint and caret blink do not cause idle msg to be sent
	return msg.message != WM_PAINT && msg.message != 0x0118;
}


int CWnd::RunModalLoop(DWORD flags)
{
	ASSERT(::IsWindow(handle()), L"expected existing window");

	bool send_idle= true;
	int idle_counter= 0;

	MSG* msg= &AppGetCurrentMessage();

	HWND parent= ::GetParent(handle());

	continue_modal_ = true;

	for (bool exit_run_loop= false; !exit_run_loop; )
	{
		while (send_idle && !::PeekMessage(msg, 0, 0, 0, PM_NOREMOVE))
		{
			if (parent && idle_counter == 0)
				::SendMessage(parent, WM_ENTERIDLE, MSGF_DIALOGBOX, reinterpret_cast<LPARAM>(handle()));

			if (/*no-kick-idle || */ !SendMessage(WM_KICKIDLE, MSGF_DIALOGBOX, idle_counter++))
				send_idle = false;
		}

		for (;;)
		{
			if (!PumpMessage())
			{
				::PostQuitMessage(0);
				return -1;
			}

			//// show the window when certain special messages rec'd
			//if (bShowIdle &&
			//	(pMsg->message == 0x118 || pMsg->message == WM_SYSKEYDOWN))
			//{
			//	ShowWindow(SW_SHOWNORMAL);
			//	UpdateWindow();
			//	bShowIdle = FALSE;
			//}

			if (!ContinueModal())
			{
				exit_run_loop = true;
				break;
			}

			if (IsIdleMessage(*msg))
			{
				send_idle = true;
				idle_counter = 0;
			}

			if (!::PeekMessage(msg, 0, 0, 0, PM_NOREMOVE))
				break;
		}
	}

	return modal_result_;
}


#define BAIL_IF_NO_WND(...)		{ if (handle() == 0) { ASSERT(false); return __VA_ARGS__; } }

HWND AppGetParentOwner(HWND hwnd)
{
	// check for permanent-owned window first
	if (CWnd* wnd= CWnd::FromHandlePermanent(hwnd))
		return wnd->GetOwner()->GetSafeHwnd();

	// otherwise, return parent in the Windows sense
	return (::GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD) ? ::GetParent(hwnd) : ::GetWindow(hwnd, GW_OWNER);
}


CWnd* CWnd::GetOwner() const
{
	return owner_wnd_;
}


void CWnd::SetOwner(CWnd* owner_wnd)
{
	owner_wnd_ = owner_wnd;
}


BOOL CWnd::IsDialogMessage(MSG* msg)
{
	ASSERT(::IsWindow(handle()));
	return ::IsDialogMessage(handle(), msg);
}


BOOL CWnd::PreTranslateInput(MSG* msg)
{
	ASSERT(::IsWindow(handle()));

	// don't translate non-input events
	if ((msg->message < WM_KEYFIRST || msg->message > WM_KEYLAST) &&
		(msg->message < WM_MOUSEFIRST || msg->message > WM_MBUTTONDBLCLK))
		return false;

	return IsDialogMessage(msg);
}


BOOL CWnd::PreTranslateMessage(MSG* msg)
{
	return false;
}


CWnd* CWnd::GetTopLevelParent() const
{
	if (GetSafeHwnd() == 0)
		return 0;

//	ASSERT_VALID(this);

	HWND parent= handle();

	for (HWND h= 0; ; )
	{
		h = AppGetParentOwner(parent);
		if (h)
			parent = h;
		else
			break;
	}

	return CWnd::FromHandle(parent);
}


LRESULT CWnd::SendMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{
	BAIL_IF_NO_WND(0);
	return ::SendMessage(handle(), msg, wparam, lparam);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CWnd::OnSize(UINT type, int cx, int cy)
{
	BAIL_IF_NO_WND();
	Default();
}


void CWnd::OnPaint()
{
	BAIL_IF_NO_WND();
	Default();
}


BOOL CWnd::OnEraseBkgnd(CDC* dc)
{
	BAIL_IF_NO_WND(false);
	return Default();
}


void CWnd::OnDrawItem(int id, DRAWITEMSTRUCT* draw_item)
{
	//TODO
	if (draw_item->CtlType == ODT_MENU)
	{
	//	CMenu* pMenu = CMenu::FromHandlePermanent(
	//		(HMENU)lpDrawItemStruct->hwndItem);
	//	if (pMenu != NULL)
	//	{
	//		pMenu->DrawItem(lpDrawItemStruct);
	//		return; // eat it
	//	}
	}

	// send notification to child control
	if (ReflectLastMsg(draw_item->hwndItem))
		return;     // handled

	Default();
}


void CWnd::OnMeasureItem(int id, MEASUREITEMSTRUCT* measure_item)
{
	if (measure_item->CtlType == ODT_MENU)
	{
/*
		ASSERT(lpMeasureItemStruct->CtlID == 0);
		CMenu* pMenu=NULL;

		_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
		if (pThreadState->m_hTrackingWindow == m_hWnd)
		{
			// start from popup
			pMenu = CMenu::FromHandle(pThreadState->m_hTrackingMenu);
		}
		else
		{
			// start from menubar
			pMenu = GetMenu();
		}

		ENSURE(pMenu);

		pMenu = _AfxFindPopupMenuFromID(pMenu, lpMeasureItemStruct->itemID);
		if (pMenu != NULL)
		{
			pMenu->MeasureItem(lpMeasureItemStruct);
		}
		else
		{
			TRACE(traceAppMsg, 0, "Warning: unknown WM_MEASUREITEM for menu item 0x%04X.\n",
				lpMeasureItemStruct->itemID);
		}
*/
	}
	else
	{
		if (CWnd* child= GetDescendantWindow(measure_item->CtlID, true))
			if (child->SendChildNotifyLastMsg())
				return;     // handled
	}

	Default();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWnd* CWnd::GetDescendantWindow(int id, bool only_permanent) const
{
	return CWnd::GetDescendantWindow(handle(), id, only_permanent);
}


CWnd* CWnd::GetDescendantWindow(HWND hwnd, int id, bool only_permanent)
{
	ASSERT(hwnd != 0);

	if (hwnd == 0)
		return 0;

	if (HWND hwnd_child= ::GetDlgItem(hwnd, id))
	{
		if (::GetTopWindow(hwnd_child) != NULL)
			if (CWnd* ctrl= GetDescendantWindow(hwnd_child, id, only_permanent))
				return ctrl;

		if (!only_permanent)	// is temporary allowed?
			return CWnd::FromHandle(hwnd_child);

		// return permanent object if there is one
		if (CWnd* ctrl= CWnd::FromHandlePermanent(hwnd_child))
			return ctrl;
	}

	for (HWND hwnd_child= ::GetTopWindow(hwnd); hwnd_child; hwnd_child = ::GetNextWindow(hwnd_child, GW_HWNDNEXT))
		if (CWnd* ctrl= GetDescendantWindow(hwnd_child, id, only_permanent))
			return ctrl;

	return 0;	// not found
}


void CWnd::Invalidate(BOOL erase) const
{
	BAIL_IF_NO_WND();
	::InvalidateRect(handle(), 0, erase);
}


CWnd* CWnd::GetDlgItem(int id) const
{
	BAIL_IF_NO_WND(0);
	if (HWND ctrl= ::GetDlgItem(handle(), id))
		return CWnd::FromHandle(ctrl);
	else
		return 0;
}


BOOL CWnd::SubclassDlgItem(UINT id, CWnd* parent)
{
	ASSERT(handle() == 0, L"window is already in use");

	HWND p= parent->GetSafeHwnd();
	if (p == 0)
		return false;

	if (HWND ctrl= ::GetDlgItem(p, id))
		return SubclassWindow(ctrl);

	return false;
}


void CWnd::CheckRadioButton(int idFirstButton, int idLastButton, int idCheckButton)
{
	BAIL_IF_NO_WND();
	::CheckRadioButton(handle(), idFirstButton, idLastButton, idCheckButton);
}


void CWnd::GetWindowRect(RECT* rect) const
{
	BAIL_IF_NO_WND();
	::GetWindowRect(handle(), rect);
}


void CWnd::GetClientRect(RECT* rect) const
{
	BAIL_IF_NO_WND();
	::GetClientRect(handle(), rect);
}


CRect CWnd::ClientRect() const
{
	CRect client;
	BAIL_IF_NO_WND(client);
	::GetClientRect(handle(), client);
	return client;
}


CRect CWnd::GetBounds() const
{
	CRect bounds;
	BAIL_IF_NO_WND(bounds);
	if (::GetWindowRect(handle(), bounds))
		if (HWND parent= ::GetParent(handle()))
			::MapWindowPoints(HWND_DESKTOP, parent, reinterpret_cast<POINT*>(static_cast<RECT*>(&bounds)), 2);
	return bounds;
}


CSize CWnd::GetSize() const
{
	BAIL_IF_NO_WND(CSize());
	CRect bounds;
	::GetWindowRect(handle(), bounds);
	return bounds.Size();
}


void CWnd::SetBounds(CRect rect)
{
	BAIL_IF_NO_WND();
	::SetWindowPos(handle(), 0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}


void CWnd::SetSize(CSize size)
{
	BAIL_IF_NO_WND();
	::SetWindowPos(handle(), 0, 0, 0, size.cx, size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}


void CWnd::SetWidth(int width)
{
	BAIL_IF_NO_WND();
	CSize size= GetSize();
	size.cx = width;
	SetSize(size);
}


void CWnd::SetHeight(int height)
{
	BAIL_IF_NO_WND();
	CSize size= GetSize();
	size.cy = height;
	SetSize(size);
}


BOOL CWnd::GetWindowPlacement(WINDOWPLACEMENT* wndpl) const
{
	BAIL_IF_NO_WND(false);
	if (wndpl == 0)
		return false;

	wndpl->length = sizeof(*wndpl);
	return ::GetWindowPlacement(handle(), wndpl);
}


BOOL CWnd::SetWindowPlacement(const WINDOWPLACEMENT* wndpl)
{
	BAIL_IF_NO_WND(false);
//	wndpl->length = sizeof(*wndpl);
	return ::SetWindowPlacement(handle(), wndpl);
}


BOOL CWnd::SetWindowPos(const CWnd* wndInsertAfter, int x, int y, int cx, int cy, UINT flags)
{
	BAIL_IF_NO_WND(false);
	return ::SetWindowPos(handle(), wndInsertAfter->GetSafeHwnd(), x, y, cx, cy, flags);
}


HICON CWnd::SetIcon(HICON hicon, BOOL bigIcon)
{
	BAIL_IF_NO_WND(0);
	return reinterpret_cast<HICON>(::SendMessage(handle(), WM_SETICON, bigIcon, reinterpret_cast<LPARAM>(hicon)));
}


int CWnd::GetDlgItemText(int id, CString& string) const
{
	BAIL_IF_NO_WND(0);

	string.clear();

	if (HWND ctrl= ::GetDlgItem(handle(), id))
	{
		int len= ::GetWindowTextLength(ctrl);
		if (len > 0)
		{
			string.resize(static_cast<size_t>(len));
			::GetWindowText(ctrl, const_cast<TCHAR*>(string.data()), len + 1);
		}
	}

	return string.int_length();
}


void CWnd::OnNcDestroy()
{
	BAIL_IF_NO_WND();

	//TODO:
	// post quit when closing main wnd of current thread


	PostNcDestroy();
}


BOOL CWnd::DestroyWindow()
{
	BAIL_IF_NO_WND(false);

	//TODO: exclude special window vars if needed (topmost, bottommost, etc.)
	BOOL ret= ::DestroyWindow(Detach());

	return ret;
}


DWORD CWnd::GetStyle() const
{
	BAIL_IF_NO_WND(0);
	return ::GetWindowLong(handle(), GWL_STYLE);
}


DWORD CWnd::GetExStyle() const
{
	BAIL_IF_NO_WND(0);
	return ::GetWindowLong(handle(), GWL_EXSTYLE);
}


static bool ModifyWndStyle(HWND hwnd, DWORD style, DWORD remove, DWORD add, int index, UINT flags)
{
	DWORD new_style= (style & ~remove) | add;
	if (new_style == style)
		return false;

	::SetWindowLong(hwnd, index, new_style);

	if (flags)
		::SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | flags);

	return true;
}


BOOL CWnd::ModifyStyle(DWORD remove, DWORD add, UINT flags)
{
	BAIL_IF_NO_WND(false);
	return ModifyWndStyle(handle(), GetStyle(), remove, add, GWL_STYLE, flags);
}


BOOL CWnd::ModifyStyleEx(DWORD remove, DWORD add, UINT flags)
{
	BAIL_IF_NO_WND(false);
	return ModifyWndStyle(handle(), GetExStyle(), remove, add, GWL_EXSTYLE, flags);
}


BOOL CWnd::OnCommand(WPARAM wparam, LPARAM lparam)
{
	BAIL_IF_NO_WND(false);

	int code= HIWORD(wparam);
	uint32 id= LOWORD(wparam);

	if (HWND ctrl= reinterpret_cast<HWND>(lparam))
	{
		// if window handle is present, this is notification from control window

		if (global_disable_notifications == handle())
			return true;	// notification handling is blocked for this window

		// reflect msg to the control (we are at the parent level)
		if (ReflectLastMsg(ctrl))
			return true;    // handled

	}
	else
	{
		// regular command

		// make sure command has not become disabled before routing
		//TODO:
		//CTestCmdUI state;
		//state.m_nID = id;
		//OnCmdMsg(nID, NC_UPDATE_COMMAND_UI, &state, NULL);
		//if (!state.m_bEnabled)
		//{
		//	TRACE(traceAppMsg, 0, "Warning: not executing disabled command %d\n", nID);
		//	return true;
		//}

		// menu or accelerator
		code = NC_COMMAND;
	}

	// dispatch through the message map
	return OnCmdMsg(id, code, WM_COMMAND, 0, 0);
}


BOOL CWnd::OnNotify(WPARAM wparam, LPARAM lparam, LRESULT* result)
{
	BAIL_IF_NO_WND(false);

	//LRESULT temp= 0;
	//if (result == 0)
	//	resut = &temp;

//	ASSERT(pResult != NULL);
	NMHDR* hdr= reinterpret_cast<NMHDR*>(lparam);
	HWND ctrl= hdr->hwndFrom;

	UINT id= ctrl ? ::GetDlgCtrlID(ctrl) : 0;
	int code= hdr->code;

	ASSERT(ctrl != NULL);
	ASSERT(::IsWindow(ctrl));

	if (global_disable_notifications == handle())
		return true;		// notification handling is blocked for this window

	// first check if child window is interested in this notification
	if (CWnd::ReflectLastMsg(ctrl, result))
		return true;        // handled by child window

	// let parent (this) window handle it
	return OnCmdMsg(id, code, WM_NOTIFY, hdr, result);
}


extern HWND find_first_radio_button(HWND button);


bool CWnd::ReflectLastMsg(HWND wnd_child, LRESULT* result)
{
//TCHAR m[60];
//wsprintf(m, L"w: %x\n", wnd_child);
//TRACE(m);

	if (CWnd* ctrl= CWnd::FromHandlePermanent(wnd_child))
	{
		ASSERT(ctrl->handle() == wnd_child);
		return ctrl->SendChildNotifyLastMsg(result);
	}
	else if (::SendMessage(wnd_child, WM_GETDLGCODE, 0, 0) & DLGC_RADIOBUTTON)
	{
		const MSG* m= GetCurrentMessage();
		if (m->message == WM_COMMAND && HIWORD(m->wParam) == BN_CLICKED)
		{
			// deal with radio buttons that don't have corresponding CWnd-based instances created;
			// find first radio button in a group; if it has a CWnd-based instance, let it know about the click
			HWND first_button= find_first_radio_button(wnd_child);
			if (CWnd* ctrl= CWnd::FromHandlePermanent(first_button))
			{
				ASSERT(ctrl->handle() == first_button);
				return ctrl->OnChildNotify(WM_COMMAND, BN_UNPUSHED << 16, 0, result) != 0;
			}
		}
	}

	return false;
}


bool CWnd::SendChildNotifyLastMsg(LRESULT* result)
{
	const MSG* m= GetCurrentMessage();
	return OnChildNotify(m->message, m->wParam, m->lParam, result) != 0;
}

// this virtual function is given opportunity to delegate some messages received by parent (this window)
// back to its children; this is message reflection and it allows classes associated with child controls
// to handle certain messages without bothering parent windows
BOOL CWnd::OnChildNotify(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result)
{
	return ReflectChildNotify(message, wparam, lparam, result);
}


bool CWnd::ReflectChildNotify(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result)
{
	switch (message)
	{
	case WM_HSCROLL:
	case WM_VSCROLL:
	case WM_PARENTNOTIFY:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_DELETEITEM:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
	case WM_COMPAREITEM:
		// those messages are sent back to the child wnd proc routine to have a chance to go through
		// child's message map before parent sees them; if handled by a child, parent won't see them
		return CWnd::OnWndMsg(WM_REFLECT_FLAG | message, wparam, lparam, result) != 0;

	// special case for WM_COMMAND
	case WM_COMMAND:
		{
			// reflect the message through the message map as OCM_COMMAND
			int code= HIWORD(wparam);
			if (CWnd::OnCmdMsg(0, code, WM_REFLECT_FLAG | WM_COMMAND, 0, result))
			{
				if (result)
					*result = 1;
				return true;
			}
		}
		break;

	// special case for WM_NOTIFY
	case WM_NOTIFY:
		{
			NMHDR* hdr= reinterpret_cast<NMHDR*>(lparam);
			return CWnd::OnCmdMsg(0, hdr->code, WM_REFLECT_FLAG | WM_NOTIFY, hdr, result);
		}

	// other special cases
	default:
/*		if (message >= WM_CTLCOLORMSGBOX && message <= WM_CTLCOLORSTATIC)
		{
			// fill in special struct for compatiblity with 16-bit WM_CTLCOLOR
			AFX_CTLCOLOR ctl;
			ctl.hDC = (HDC)wParam;
			ctl.nCtlType = message - WM_CTLCOLORMSGBOX;
			//ASSERT(ctl.nCtlType >= CTLCOLOR_MSGBOX);
			ASSERT(ctl.nCtlType <= CTLCOLOR_STATIC);

			// reflect the message through the message map as OCM_CTLCOLOR
			BOOL bResult = CWnd::OnWndMsg(WM_REFLECT_BASE+WM_CTLCOLOR, 0, (LPARAM)&ctl, pResult);
			if ((HBRUSH)*pResult == NULL)
				bResult = FALSE;
			return bResult;
		}*/
		break;
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////


BOOL CWnd::ShowWindow(int cmd_show)
{
	BAIL_IF_NO_WND(false);
	return ::ShowWindow(handle(), cmd_show);
}


void CWnd::HideWnd()
{
	SetWindowPos(0, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}


void CWnd::ShowWnd()
{
	SetWindowPos(0, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}


void CWnd::ShowActivateWnd()
{
	ShowWindow(SW_SHOW);
}


CString AfxRegisterWndClass(UINT classStyle, HCURSOR cursor, HBRUSH background_brush, HICON icon)
{
	TCHAR name[200];

	HINSTANCE instance= AfxGetInstanceHandle();

	if (cursor == 0 && background_brush == 0 && icon == 0)
		_sntprintf_s(name, array_count(name), array_count(name) - 1, _T("Micron:%p:%x"), instance, classStyle);
	else
		_sntprintf_s(name, array_count(name), array_count(name) - 1, _T("Micron:%p:%x:%p:%p:%p"), instance, classStyle, cursor, background_brush, icon);

	// does it exist?
	WNDCLASS wndcls;
	if (::GetClassInfo(instance, name, &wndcls))
		return name;

	// register it
	wndcls.style = classStyle;
	wndcls.lpfnWndProc = &::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hInstance = instance;
	wndcls.hIcon = icon;
	wndcls.hCursor = cursor;
	wndcls.hbrBackground = background_brush;
	wndcls.lpszMenuName = 0;
	wndcls.lpszClassName = name;
	if (!::RegisterClass(&wndcls))
		throw mcr::micron_exception("wnd class registration failed"); //AfxThrowResourceException();

	return name;
}


extern HINSTANCE g_Instance= 0;

//HINSTANCE AfxGetInstanceHandle()
//{
//	return g_Instance;
//}


BOOL CWnd::AttachDlgItem(UINT nID, CWnd* pParent)
{
	HWND hWnd= ::GetDlgItem(pParent->GetHwnd(), nID);
	return Attach(hWnd);
}

BOOL CWnd::BringWindowToTop() const
{
	return ::BringWindowToTop(handle());
}

BOOL CWnd::CloseWindow() const
{
	return ::CloseWindow(handle());
}

HDWP CWnd::DeferWindowPos(HDWP hWinPosInfo, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
{
	return ::DeferWindowPos(hWinPosInfo, handle(), hWndInsertAfter, x, y, cx, cy, uFlags);
}

HDWP CWnd::DeferWindowPos(HDWP hWinPosInfo, HWND hWndInsertAfter, RECT rc, UINT uFlags)
{
	return ::DeferWindowPos(hWinPosInfo, handle(), hWndInsertAfter, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, uFlags);
}


//BOOL CWnd::Destroy()
//{
//	if (IsWindow()) DestroyWindow();
//
//	handle_.set(0);
//
//	return true;
//}


BOOL CWnd::DrawMenuBar() const
{
	return ::DrawMenuBar(handle());
}


BOOL CWnd::EnableWindow(BOOL bEnable /*= TRUE*/) const
{
	return ::EnableWindow(handle(), bEnable);
}

//inline HWND CWnd::GetAncestor(HWND hWnd) const
//{
//	// Returns the root parent.  Supports Win95
//	HWND hWndParent = ::GetParent(hWnd);
//	while (::IsChild(hWndParent, hWnd))
//	{
//		hWnd = hWndParent;
//		hWndParent = ::GetParent(hWnd);
//	}

//	return hWnd;
//}

ULONG_PTR CWnd::GetClassLongPtr(int nIndex) const
{
	return ::GetClassLongPtr(handle(), nIndex);
}

HDC CWnd::GetDC() const
{
	return ::GetDC(handle());
}

HDC CWnd::GetDCEx(HRGN hrgnClip, DWORD flags) const
{
	return ::GetDCEx(handle(), hrgnClip, flags);
}

HWND CWnd::GetDlgHItem(int nIDDlgItem) const

{
	return ::GetDlgItem(handle(), nIDDlgItem);
}


HMENU CWnd::GetMenu() const
{
	return ::GetMenu(handle());
}

LONG_PTR CWnd::GetWindowLongPtr(int nIndex) const
{
	return ::GetWindowLongPtr(handle(), nIndex);
}

BOOL CWnd::GetScrollInfo(int fnBar, SCROLLINFO& si) const
{
	return ::GetScrollInfo(handle(), fnBar, &si);
}

int CWnd::GetScrollPos(int nBar) const
{
	return ::GetScrollPos(handle(), nBar);
}

BOOL CWnd::GetScrollRange(int nBar, int& MinPos, int& MaxPos) const
{
	return ::GetScrollRange(handle(), nBar, &MinPos, &MaxPos );
}

HWND CWnd::GetWindow(UINT uCmd) const
{
	return ::GetWindow(handle(), uCmd);
}

HDC CWnd::GetWindowDC() const
{
	return ::GetWindowDC(handle());
}


CRect CWnd::GetWindowRect() const
{
	CRect rc;
	::GetWindowRect(handle(), &rc);
	return rc;
}

BOOL CWnd::InvalidateRect(CONST RECT* lpRect, BOOL bErase /*= TRUE*/) const
{
	return ::InvalidateRect(handle(), lpRect, bErase);
}

BOOL CWnd::InvalidateRgn(CONST HRGN hRgn, BOOL bErase /*= TRUE*/) const
{
	return ::InvalidateRgn(handle(), hRgn, bErase);
}

BOOL CWnd::IsChild(const CWnd* pWndParent) const
{
	return ::IsChild(pWndParent->GetHwnd(), handle());
}

BOOL CWnd::IsEnabled() const
{
	return ::IsWindowEnabled(handle());
}

BOOL CWnd::IsIconic() const
{
	return ::IsIconic(handle());
}

BOOL CWnd::IsWindowVisible() const
{
	return ::IsWindowVisible(handle());
}

BOOL CWnd::IsWindow() const
{
	return ::IsWindow(handle());
}

BOOL CWnd::IsZoomed() const
{
	return ::IsZoomed(handle());
}

HBITMAP CWnd::LoadBitmap(LPCTSTR lpBitmapName) const
{
	if (0 == GetApp())
		throw CWinException(_T("LoadBitmap ... Win32++ has not been initialised successfully."));

	HBITMAP hBitmap;

	// Try to load the bitmap from the resource handle first
	hBitmap = ::LoadBitmap(GetApp()->GetResourceHandle(), lpBitmapName);

	// The bitmap resource might be in the application's resources instead
	if (!hBitmap)
		hBitmap = ::LoadBitmap(GetApp()->GetInstanceHandle(), lpBitmapName);

	// No bitmap found, so display warning message
	if (!hBitmap)
		TRACE(_T("Unable to load bitmap\n"));

	return hBitmap;
}


int CWnd::OnCreate(CREATESTRUCT* create_struct)
{
	return Default();
}


int CWnd::MessageBox(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) const
{
	return ::MessageBox(handle(), lpText, lpCaption, uType);
}

/*
LRESULT CWnd::MessageReflect(HWND hWndParent, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// A function used to call OnMessageReflect. You shouldn't need to call or
	//  override this function.

	HWND hWnd = NULL;
	switch (uMsg)
	{
	case WM_COMMAND:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
	case WM_CHARTOITEM:
	case WM_VKEYTOITEM:
	case WM_HSCROLL:
	case WM_VSCROLL:
		hWnd = (HWND)lParam;
		break;

	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_DELETEITEM:
	case WM_COMPAREITEM:
		hWnd = ::GetDlgItem(hWndParent, (int)wParam);
		break;

	case WM_PARENTNOTIFY:
		switch(LOWORD(wParam))
		{
		case WM_CREATE:
		case WM_DESTROY:
			hWnd = (HWND)lParam;
			break;
		}
	}

	CWnd* Wnd = FromHandle(hWnd);

	if (Wnd != NULL)
		return Wnd->OnMessageReflect(uMsg, wParam, lParam);

	return 0L;
}
*/
void CWnd::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint /* = TRUE*/) const
{
	::MoveWindow(handle(), x, y, nWidth, nHeight, bRepaint = TRUE);
}

void CWnd::MoveWindow(CRect& rc, BOOL bRepaint /* = TRUE*/) const
{
	::MoveWindow(handle(), rc.left, rc.top, rc.Width(), rc.Height(), bRepaint);
}

//inline LRESULT CWnd::OnMessageReflect(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
//{
//	return 0L;
//}

//inline LRESULT CWnd::OnNotifyReflect(WPARAM /*wParam*/, LPARAM /*lParam*/)
//{
//	return 0L;
//}

BOOL CWnd::PostMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/) const
{
	return ::PostMessage(handle(), uMsg, wParam, lParam);
}

BOOL CWnd::RedrawWindow(CRect* lpRectUpdate, HRGN hRgn, UINT flags) const
{
	return ::RedrawWindow(handle(), lpRectUpdate, hRgn, flags);
}


int CWnd::ReleaseDC(HDC hDC) const
{
	return ::ReleaseDC(handle(), hDC);
}


BOOL CWnd::ScrollWindow(int dx, int dy, const RECT* rect, const RECT* clip)
{
	return ::ScrollWindow(handle(), dx, dy, rect, clip);
}


HWND CWnd::SetActiveWindow()
{
	return ::SetActiveWindow(handle());
}

HWND CWnd::SetCapture()
{
	return ::SetCapture(handle());
}

ULONG_PTR CWnd::SetClassLongPtr(int nIndex, LONG_PTR dwNewLong)
{
	return ::SetClassLongPtr(handle(), nIndex, dwNewLong);
}

HWND CWnd::SetFocus()
{
	return ::SetFocus(handle());
}

BOOL CWnd::SetForegroundWindow()
{
	return ::SetForegroundWindow(handle());
}

BOOL CWnd::SetMenu(HMENU hMenu)
{
	return ::SetMenu(handle(), hMenu);
}

void CWnd::SetParent(HWND hParent)
{
	if ((0 == hParent) || (::IsWindow(hParent)))
	{
		//m_hWndParent = hParent;
		if (::IsWindow(handle()))
			::SetParent(handle(), hParent);
	}
//		else
//			throw CWinException(_T("CWnd::SetParent ... Failed to set parent"));

}


BOOL CWnd::SetRedraw(BOOL bRedraw /*= TRUE*/)
{
	return ::SendMessage(handle(), WM_SETREDRAW, (WPARAM)bRedraw, 0);
}

int CWnd::SetScrollInfo(int fnBar, SCROLLINFO& si, BOOL fRedraw)
{
	return ::SetScrollInfo(handle(), fnBar, &si, fRedraw);
}

int CWnd::SetScrollPos(int nBar, int nPos, BOOL bRedraw)
{
	return ::SetScrollPos(handle(), nBar, nPos, bRedraw);
}

BOOL CWnd::SetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL bRedraw)
{
	return ::SetScrollRange(handle(), nBar, nMinPos, nMaxPos, bRedraw);
}

LONG_PTR CWnd::SetWindowLongPtr(int nIndex, LONG_PTR dwNewLong)
{
	return ::SetWindowLongPtr(handle(), nIndex, dwNewLong);
}

BOOL CWnd::SetWindowPos(HWND hWndInsertAfter, RECT rc, UINT uFlags)
{
	return ::SetWindowPos(handle(), hWndInsertAfter, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, uFlags);
}

int CWnd::SetWindowRgn(HRGN hRgn, BOOL bRedraw /*= TRUE*/)
{
	return ::SetWindowRgn(handle(), hRgn, bRedraw);
}

BOOL CWnd::SetWindowText(LPCTSTR lpString)
{
	return ::SetWindowText(handle(), lpString);
}

BOOL CWnd::UpdateWindow() const
{
	return ::UpdateWindow(handle());
}

BOOL CWnd::ValidateRect(CRect& rc) const
{
	return ::ValidateRect(handle(), &rc);
}

BOOL CWnd::ValidateRgn(HRGN hRgn) const
{
	return ::ValidateRgn(handle(), hRgn);
}


UINT_PTR CWnd::SetTimer(UINT_PTR event_id, UINT elapse, void (CALLBACK* fn_timer)(HWND, UINT, UINT_PTR, DWORD))
{
	//TODO
	ASSERT(0);
	return 0;
}

BOOL CWnd::KillTimer(UINT_PTR event_id)
{
	//TODO
	ASSERT(0);
	return false;
}


void CWnd::OnTimer(UINT_PTR event_id)
{
	Default();
}

void CWnd::OnLButtonDown(UINT flags, CPoint point)
{
	Default();
}

CWnd* CWnd::GetDesktopWindow()
{
	return CWnd::FromHandle(::GetDesktopWindow());
}


BOOL CWnd::IsFrameWnd() const
{
	ASSERT(0);
	//TODO
	return false;
}


} // namespace


// global scope

CString AfxRegisterWndClass(UINT classStyle, HCURSOR cursor, HBRUSH background_brush, HICON icon)
{
	return mcr::AfxRegisterWndClass(classStyle, cursor, background_brush, icon);
}
