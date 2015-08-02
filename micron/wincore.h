#pragma once

#ifndef WINCORE_H
#define WINCORE_H


#ifndef WM_PARENTNOTIFY
# define WM_PARENTNOTIFY 0x0210
#endif


#include "size-point-rect.h"
#include "handle_map.h"
#include "cmdtarget.h"

namespace mcr
{
	class CDC;
	class CFrameWnd;
	class CView;
	class CScrollBar;
	class CFont;

	inline void TRACE(LPCTSTR str)
	{
	#ifdef _DEBUG
		OutputDebugString(str);
	#else
		UNREFERENCED_PARAMETER(str); // no-op
	#endif
	}

	class CWinApp;
	class CWnd;

	enum Constants
	{
		MAX_STRING_SIZE = 255,
	};

	CWinApp* GetApp();

	// Displays an error message in a message box. Debug mode only.
	inline void DebugWarnMsg(LPCTSTR WarnMsg)
	{
	#ifdef _DEBUG
		::MessageBox (0, WarnMsg, _T("Warning"), MB_ICONINFORMATION | MB_OK);
	#else
		UNREFERENCED_PARAMETER(WarnMsg); // no-op
	#endif  //_DEBUG
	}

	// Displays a warning message in a messagebox. Debug mode only
	inline void DebugErrMsg(LPCTSTR ErrorMsg)
	{
	#ifdef _DEBUG
		::MessageBox (0, ErrorMsg, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	#else
		UNREFERENCED_PARAMETER(ErrorMsg); // no-op
	#endif  //_DEBUG
	}


	class CException
	{
	public:
		CException()
		{}
		virtual ~CException() {}
		void ReportError() {}
		void Delete() {}
	};

	////////////////////////////////////////
	// Declaration of the CWinException class
	//
	class CWinException : public CException
	{
	public:
		CWinException (LPCTSTR msg) : m_err (::GetLastError()), m_msg(msg) {}
		LPCTSTR What() const {return m_msg;}
		void MessageBox() const
		{
			TCHAR buf1 [MAX_STRING_SIZE/2 -10] = _T("");
			TCHAR buf2 [MAX_STRING_SIZE/2 -10] = _T("");
			TCHAR buf3 [MAX_STRING_SIZE]       = _T("");

			lstrcpyn(buf1, m_msg, MAX_STRING_SIZE/2 -10);

			// Display Last Error information if it's useful
			if (m_err != 0)
			{
				::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, m_err,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf2, MAX_STRING_SIZE/2 -10, NULL);

				::wsprintf(buf3, _T("%s\n\n     %s\n\n"), buf1, buf2);
			}
			else
				::wsprintf(buf3, _T("%s"), buf1);

			::MessageBox (0, buf3, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		}

	private:
		DWORD  m_err;
		LPCTSTR m_msg;

	};


	class CMenu;


class CCmdUI	// update UI
{
public:
	UINT m_nID;
	UINT m_nIndex;          // menu item or other index

	// if a menu item
	CMenu* m_pMenu;         // NULL if not a menu
	CMenu* m_pSubMenu;      // sub containing menu item
							// if a popup sub menu - ID is for first in popup

	// if from some other window
	CWnd* m_pOther;         // NULL if a menu or not a CWnd

// Operations to do in ON_UPDATE_COMMAND_UI
	virtual void Enable(BOOL bOn = TRUE);
	virtual void SetCheck(int nCheck = 1);   // 0, 1 or 2 (indeterminate)
	virtual void SetRadio(BOOL bOn = TRUE);
	virtual void SetText(LPCTSTR lpszText);

// Advanced operation
	void ContinueRouting();

// Implementation
	CCmdUI();
	BOOL m_bEnableChanged;
	BOOL m_bContinueRouting;
	UINT m_nIndexMax;       // last + 1 for iterating m_nIndex

	CMenu* m_pParentMenu;   // NULL if parent menu not easily determined
							//  (probably a secondary popup menu)

	BOOL DoUpdate(CCmdTarget* pTarget, BOOL bDisableIfNoHndler);
};


class CDataExchange;


struct CCreateContext
{
	CCreateContext() : current_frame_(0)
	{}

	CFrameWnd* current_frame_;
	boost::function<CView* ()> create_view_;
};


class CWnd : public CCmdTarget
{
public:
	CWnd();
	virtual ~CWnd();

	virtual BOOL Create(const TCHAR* className, const TCHAR* windowName, DWORD style, const RECT& rect, CWnd* parentWnd, UINT id, CCreateContext* context= 0);
//	virtual BOOL Destroy();

	virtual BOOL PreTranslateMessage(MSG* msg);

	BOOL AttachDlgItem(UINT id, CWnd* parent);
	BOOL BringWindowToTop() const;
	void CenterWindow() const;
	BOOL CreateEx(DWORD exStyle, const TCHAR* className, const TCHAR* windowName, DWORD style, int x, int y, int width, int height, HWND parent, HMENU id_or_menu, void* param);
	BOOL CreateEx(DWORD exStyle, const TCHAR* className, const TCHAR* windowName, DWORD style, const RECT& rect, CWnd* parentWnd, UINT id, void* param);

	LRESULT DefWindowProc(UINT msg, WPARAM wParam, LPARAM lParam);
	HDWP DeferWindowPos(HDWP win_pos_info, HWND wnd_insert_after, int x, int y, int cx, int cy, UINT flags);
	HDWP DeferWindowPos(HDWP win_pos_info, HWND wnd_insert_after, RECT rc, UINT flags);
	BOOL DestroyWindow();
	BOOL DrawMenuBar() const;
	BOOL EnableWindow(BOOL enable = TRUE) const;
	HWND GetAncestor(HWND wnd) const;
	ULONG_PTR GetClassLongPtr(int index) const;

CRect ClientRect() const;
	void GetClientRect(RECT* rect) const;
	HDC  GetDC() const;
	HDC  GetDCEx(HRGN hrgnClip, DWORD flags) const;

CRect GetBounds() const;
CSize GetSize() const;
void SetBounds(CRect rect);
void SetSize(CSize size);
void SetWidth(int width);
void SetHeight(int height);

	int GetDlgCtrlID() const;
	int SetDlgCtrlID(int id);

	void GetDlgItem(int id_dlg_item, HWND* hwnd) const;
	CWnd* GetDlgItem(int id_dlg_item) const;
	HWND GetDlgHItem(int id_dlg_item) const;
	void UpdateDialogControls(CCmdTarget* target, bool disable_if_no_handler);

	int GetDlgItemText(int id, CString& text) const;
	// recursive:
CWnd* GetDescendantWindow(int id, bool only_permanent= false) const;
	HWND GetHwnd() const {return handle();}
	BOOL GetScrollInfo(int fnBar, SCROLLINFO& si) const;

	void SetFont(CFont* font, bool redraw= true);
	CFont* GetFont() const;

	UINT IsDlgButtonChecked(int button_id) const;
	void CheckDlgButton(int button_id, UINT check);
	void CheckRadioButton(int first_button_id, int last_button_id, int check_button_id);

	void PrintClient(CDC* dc, DWORD flags) const;

	HWND GetWindow(UINT cmd) const;
	HDC  GetWindowDC() const;
	LONG_PTR GetWindowLongPtr(int index) const;
	CRect GetWindowRect() const;
	void GetWindowRect(RECT* rect) const;
	//tString GetWindowString() const;
	void Invalidate(BOOL erase = TRUE) const;
	BOOL InvalidateRect(CONST RECT* rect, BOOL erase = TRUE) const;
	BOOL InvalidateRgn(CONST HRGN rgn, BOOL erase = TRUE) const;
	BOOL IsChild(const CWnd* wnd_parent) const;
	BOOL IsEnabled() const;
	BOOL IsWindowVisible() const;
	BOOL IsWindow() const;
	HBITMAP LoadBitmap(LPCTSTR bitmap_name) const;
	LPCTSTR LoadString(UINT id);
	int  MessageBox(LPCTSTR text, LPCTSTR caption, UINT type) const;
	LRESULT MessageReflect(HWND parent, UINT msg, WPARAM wParam, LPARAM lParam);
	void MoveWindow(int x, int y, int width, int height, BOOL repaint = TRUE) const;
	void MoveWindow(CRect& rc, BOOL repaint = TRUE) const;
	BOOL PostMessage(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0) const;
	BOOL RedrawWindow(CRect* rect_update = NULL, HRGN rgn = NULL, UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE ) const;
	BOOL RegisterClass(WNDCLASS& wc);
	int  ReleaseDC(HDC dc) const;

	LRESULT SendMessage(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);
	HWND SetActiveWindow();
	HWND SetCapture();
	ULONG_PTR SetClassLongPtr(int index, LONG_PTR new_long);
	HWND SetFocus();
	BOOL SetForegroundWindow();

	void SetParent(HWND parent);
	BOOL SetRedraw(BOOL redraw = TRUE);
	int  SetScrollInfo(int fnBar, SCROLLINFO& si, BOOL redraw);
	LONG_PTR SetWindowLongPtr(int index, LONG_PTR new_long);
	BOOL SetWindowPos(const CWnd* insert_after_wnd, int x, int y, int cx, int cy, UINT flags);
	BOOL SetWindowPos(HWND wnd_insert_after, RECT rc, UINT flags);
	int SetWindowRgn(HRGN rgn, BOOL redraw = TRUE);
	BOOL SetWindowText(LPCTSTR string);
	void GetWindowText(CString& str) const;
	int GetWindowText(TCHAR* buffer, int max_count) const;
	int GetWindowTextLength() const;
//BOOL ShowWindow(int cmd_show = SW_SHOWNORMAL);
	BOOL UpdateWindow() const;
	BOOL ValidateRect(CRect& rc) const;
	BOOL ValidateRgn(HRGN rgn) const;

	BOOL CloseWindow() const;
	HMENU GetMenu() const;
	int  GetScrollPos(int bar) const;
	BOOL GetScrollRange(int bar, int& min_pos, int& max_pos) const;
	BOOL GetWindowPlacement(WINDOWPLACEMENT* wndpl) const;
	BOOL IsIconic() const;
	BOOL IsZoomed() const;
	BOOL SetMenu(HMENU menu);
	BOOL ScrollWindow(int dx, int dy, const RECT* rect= 0, const RECT* clip= 0);
	int  SetScrollPos(int bar, int pos, BOOL redraw= true);
	BOOL SetScrollRange(int bar, int min_pos, int max_pos, BOOL redraw);
	BOOL SetWindowPlacement(const WINDOWPLACEMENT* wndpl);

	UINT_PTR SetTimer(UINT_PTR event_id, UINT elapse, void (CALLBACK* fn_timer)(HWND, UINT, UINT_PTR, DWORD));
	BOOL KillTimer(UINT_PTR event_id);

	operator HWND() const {return handle();}

	// Required by some macros
	BOOL PostMessage(HWND wnd, UINT msg, WPARAM wParam = 0, LPARAM lParam = 0) const
		{return ::PostMessage(wnd, msg, wParam, lParam);}
	LRESULT SendMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) const
		{ return ::SendMessage(wnd, msg, wParam, lParam);}

	HICON SetIcon(HICON hicon, BOOL bigIcon);
	BOOL SubclassDlgItem(UINT id, CWnd* parent);

	HWND handle() const		{ return handle_.get(); }
DWORD GetStyle() const;
DWORD GetExStyle() const;
BOOL ModifyStyle(DWORD remove, DWORD add, UINT flags = 0);
BOOL ModifyStyleEx(DWORD remove, DWORD add, UINT flags = 0);
HWND GetSafeHwnd() const;
void HideWnd();
void ShowWnd();
void ShowActivateWnd();
BOOL ShowWindow(int cmd_show);

int RunModalLoop(DWORD flags= 0);
virtual BOOL ContinueModal();
virtual void EndModalLoop(int result);
static bool WalkPreTranslateTree(HWND last_wnd, MSG& msg);
BOOL IsDialogMessage(MSG* msg);
BOOL PreTranslateInput(MSG* msg);	//TODO: make virtual

CWnd* GetParent() const;
CWnd* SetParent(CWnd* new_parent);
CWnd* GetTopLevelParent() const;
CWnd* GetOwner() const;
void SetOwner(CWnd* owner_wnd);

bool SubclassWindow(HWND hwnd);
bool CreateDlgIndirect(const DLGTEMPLATE* dialog_template, CWnd* parent_wnd, HINSTANCE instance);
bool Attach(HWND wnd);
HWND Detach();
static CWnd* FromHandle(HWND hwnd);
static CWnd* FromHandlePermanent(HWND hwnd);

bool UpdateData(bool save_and_validate);

	static CWnd* GetDesktopWindow();

	virtual BOOL IsFrameWnd() const;
	CFrameWnd* GetParentFrame() const;

	// coordinates
	void ClientToScreen(POINT* point) const;
	void ClientToScreen(RECT* rect) const;
	void ScreenToClient(POINT* point) const;
	void ScreenToClient(RECT* rect) const;

	enum CreationParameter { CreatePermanent, CreateTemporary };
	// advanced creation
	CWnd(HWND hwnd, CreationParameter p);

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

//	virtual void OnCreate();
	virtual BOOL OnNotify(WPARAM wparam, LPARAM lparam, LRESULT* result);
	virtual BOOL OnChildNotify(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result);
	bool ReflectChildNotify(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result);
	static bool ReflectLastMsg(HWND wnd_child, LRESULT* result= 0);
	bool SendChildNotifyLastMsg(LRESULT* result= 0);

	static const MSG* GetCurrentMessage();

	virtual LRESULT WindowProc(UINT message, WPARAM wparam, LPARAM lparam);
	virtual BOOL OnWndMsg(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result);


//	BOOL IsMDIChild() const {return FALSE;}


private:
	CWnd(const CWnd&);
	CWnd& operator = (const CWnd&);

	DECLARE_MESSAGE_MAP()

protected:
	void OnSize(UINT type, int cx, int cy);
	void OnPaint();
	BOOL OnEraseBkgnd(CDC* dc);
	LRESULT Default();
	void OnNcDestroy();
	void OnDrawItem(int id, DRAWITEMSTRUCT* draw_item);
	void OnMeasureItem(int id, MEASUREITEMSTRUCT* measure_item);
	void OnTimer(UINT_PTR event_id);
	void OnLButtonDown(UINT flags, CPoint point);
	BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	void OnShowWindow(BOOL show, UINT status);
	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	void OnMouseMove(UINT flags, CPoint point);
	int OnCreate(CREATESTRUCT* create_struct);
	int OnMouseActivate(CWnd* desktop_wnd, UINT hit_test, UINT message);
	void OnActivate(UINT state, CWnd* other_window, BOOL minimized);
	void OnActivateApp(BOOL active, DWORD thread_id);
	void OnDestroy();
	void OnGetMinMaxInfo(MINMAXINFO* mmi);
	void OnSizing(UINT side, RECT* rect);
	void OnKeyDown(UINT chr, UINT rep_count, UINT flags);
	void OnKeyUp(UINT chr, UINT rep_count, UINT flags);

	virtual void PreSubclassWindow();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();

	virtual void DoDataExchange(CDataExchange* dx);

	static CWnd* GetDescendantWindow(HWND hwnd, int id, bool only_permanent);

	void SetContinueModal();

	CWnd* owner_wnd_;

private:
	static LRESULT CallWndProc(CWnd* wnd, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK StdWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	bool DispatchWndMessage(mcr::Msg& msg, const message_map_entry& entry);
	void SetInternalWndProc();
	void clear_out_all_vars();

	WNDPROC original_wnd_proc_;	// original window procedure
	typedef handle_mixin<CWnd, HWND> Handle;
	Handle handle_;
	bool continue_modal_;
	int modal_result_;

	struct details;
};


class CWinThread : public CCmdTarget
{
public:

	DECLARE_MESSAGE_MAP()
};



}; // namespace mcr


#endif // WINCORE_H
