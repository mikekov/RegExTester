#pragma once

#define NOMINMAX

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <shlwapi.h>

#define GDIPVER 0x0110
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#include <GdiPlus.h>
namespace graphics_details = Gdiplus;

#undef min
#undef max

#pragma warning(disable: 4396)	// the inline specifier cannot be used when a friend declaration refers to a specialization of a function template

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/any.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>
#include <boost/ptr_container/ptr_unordered_map.hpp>
#include <boost/unordered_map.hpp>
#include <boost/scoped_ptr.hpp>
//#include <boost/smart_ptr.hpp>

#define BOOST_SIGNALS_NO_LIB 1
#include <boost/signals2.hpp>

#define BOOST_THREAD_NO_LIB 1
#include <boost/thread/tss.hpp>



//=========================================================================================================


namespace mcr
{
	class CWnd;
	class CDC;
	class CString;
	class CGdiObject;
	class CPen;
	class CBrush;
	class CFont;
	class CBitmap;
	class CFile;


	// template typedef...
	template <class T>
	class implementation_ptr : public boost::scoped_ptr<T>
	{
	public:
		explicit implementation_ptr(T* t) : boost::scoped_ptr<T>(t)
		{}
	};
}


#include "constdefs.h"
#include "basetypes.h"
#include "ctime.h"
#include "signals.h"
#include "syncobj.h"
#include "message_map.h"
#include "gdiobj.h"
#include "dc.h"
#include "graphics.h"
#include "winthread.h"
#include "wincore.h"
#include "scrollbar.h"
#include "dialog.h"
#include "winapp.h"
#include "message_translator.h"
#include "tooltipctrl.h"
#include "tabctrl.h"
#include "treectrl.h"
#include "listctrl.h"
#include "statusbarctrl.h"
#include "toolbarctrl.h"
#include "rebarctrl.h"
#include "framewnd.h"
#include "miniframewnd.h"
#include "imagelist.h"
#include "editctrl.h"
#include "button.h"
#include "view.h"
#include "cfile.h"
#include "combobox-ex.h"
#include "droptarget.h"


typedef mcr::CDC CDC;
typedef mcr::CClientDC CClientDC;
typedef mcr::CPaintDC CPaintDC;
typedef mcr::CCmdTarget CCmdTarget;
typedef mcr::CCreateContext CCreateContext;
typedef mcr::CWnd CWnd;
typedef mcr::CDialog CDialog;
typedef mcr::CDataExchange CDataExchange;
typedef mcr::CRect CRect;
typedef mcr::CSize CSize;
typedef mcr::CPoint CPoint;
typedef mcr::CString CString;
typedef mcr::CFont CFont;
typedef mcr::CBitmap CBitmap;
typedef mcr::CPen CPen;
typedef mcr::CBrush CBrush;
typedef mcr::CGdiObject CGdiObject;
typedef mcr::CException CException;
typedef mcr::CFile CFile;
typedef mcr::file_exception CFileException;
typedef mcr::CWinThread CWinThread;
typedef mcr::CWinApp CWinApp;
typedef mcr::CFile CFile;
typedef mcr::CScrollBar CScrollBar;
typedef mcr::CTime CTime;
typedef mcr::CTimeSpan CTimeSpan;
typedef mcr::CToolTipCtrl CToolTipCtrl;
typedef mcr::CEdit CEdit;
typedef mcr::CButton CButton;
typedef mcr::CCheckBox CCheckBox;
typedef mcr::CRadioButton CRadioButton;
typedef mcr::CTreeCtrl CTreeCtrl;
typedef mcr::CListCtrl CListCtrl;
typedef mcr::CToolBarCtrl CToolBarCtrl;
typedef mcr::CReBarCtrl CReBarCtrl;
typedef mcr::CFrameWnd CFrameWnd;
typedef mcr::CMiniFrameWnd CMiniFrameWnd;
typedef mcr::CCmdUI CCmdUI;
typedef mcr::CImageList CImageList;
typedef mcr::CMenu CMenu;
typedef mcr::CView CView;
typedef mcr::CComboBoxEx CComboBoxEx;
typedef mcr::CCriticalSection CCriticalSection;
typedef mcr::CSyncObject CSyncObject;
typedef mcr::CCriticalSection CCriticalSection;
typedef mcr::CSingleLock CSingleLock;
typedef mcr::COleDropTarget COleDropTarget;


#define m_hWnd	handle()


class CListBox : public CWnd
{
	typedef mcr::signal<void (CListBox& self, DRAWITEMSTRUCT*)> DrawItemSignal;
	typedef mcr::signal<void (CListBox& self, MEASUREITEMSTRUCT*)> MeasureItemSignal;
public:
	CListBox();

	int GetCurSel() const;
	int SetCurSel(int select);

	virtual void DrawItem(DRAWITEMSTRUCT* draw_item);
	virtual void MeasureItem(MEASUREITEMSTRUCT* measure_item);
	virtual int CompareItem(COMPAREITEMSTRUCT* compare_item);
	virtual void DeleteItem(DELETEITEMSTRUCT* delete_item);
	virtual int VKeyToItem(UINT key, UINT index);
	virtual int CharToItem(UINT key, UINT index);

	mcr::slot_connection DrawItemEvent(const DrawItemSignal::fn& handler);
	mcr::slot_connection MeasureItemEvent(const MeasureItemSignal::fn& handler);

	~CListBox();

protected:
	virtual BOOL OnChildNotify(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result);

private:
	struct Impl;
	mcr::implementation_ptr<Impl> impl_;
};


class CStatic : public CWnd
{};



void /*AFXAPI*/ DDX_Control(CDataExchange* pDX, int nIDC, CWnd& rControl);


// TODO: remove ---
class CPath
{
public:
	CPath(const TCHAR* path);

	void Append(const TCHAR* str);

	operator const TCHAR* ();

private:
	std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > str_;
};
// ------



CString AfxRegisterWndClass(UINT classStyle, HCURSOR cursor= 0, HBRUSH background_brush= 0, HICON icon= 0);

namespace mcr
{
	HINSTANCE AppGetInstanceHandle();
	HINSTANCE AppGetResourceHandle();
	HINSTANCE AppFindResourceHandle(const TCHAR* name, const TCHAR* type);
	CWnd* AppGetMainWnd();
	CWinApp* MicronGetApp();
	bool MicronExtractSubString(CString& string, const TCHAR* full_string, int sub_string, TCHAR separator);
}


inline HINSTANCE AfxGetInstanceHandle()	{ return mcr::AppGetInstanceHandle(); }
inline HINSTANCE AfxGetResourceHandle()	{ return mcr::AppGetResourceHandle(); }
inline HINSTANCE AfxFindResourceHandle(const TCHAR* name, const TCHAR* type)	{ return mcr::AppFindResourceHandle(name, type); }
inline CWnd* AfxGetMainWnd()			{ return mcr::AppGetMainWnd(); }
inline CWinApp* AfxGetApp()				{ return mcr::MicronGetApp(); }
inline bool AfxExtractSubString(CString& string, const TCHAR* full_string, int sub_string, TCHAR separator= '\n')
{ return MicronExtractSubString(string, full_string, sub_string, separator); }


#define afx_msg
