#pragma once

#ifndef DIALOG_H
#define DIALOG_H

#include "wincore.h"


namespace mcr
{
	class CDataExchange
	{
	public:
		CDataExchange(CWnd* dlg_wnd, bool save_and_validate);
//		CDataExchange();

		HWND PrepareCtrl(int id);
		HWND PrepareEditCtrl(int id);
		void Fail();

		CWnd* dlg() const;

	private:
		CWnd* dlg_;

		bool save_and_validate_;
		CWnd* dlg_wnd_;
	};

	class CDlgResizer
	{
	public:
		virtual void ResizeControls(CWnd& parent, int cx, int cy) = 0;

		enum ResizeFlag
		{
			NONE= 0,
			MOVE_H= 1,
			MOVE_V= 2,
			MOVE= MOVE_H | MOVE_V,
			RESIZE_H= 4,
			RESIZE_V= 8,
			RESIZE= RESIZE_H | RESIZE_V,

			MOVE_H_RESIZE_V= MOVE_H | RESIZE_V,
			MOVE_V_RESIZE_H= MOVE_V | RESIZE_H,
			MOVE_H_RESIZE=   MOVE_H | RESIZE,
			MOVE_V_RESIZE=   MOVE_V | RESIZE,
			MOVE_H_RESIZE_H= MOVE_H | RESIZE_H
		};

		enum HalfResizeFlag
		{
			HALF_MOVE_H= 1, HALF_MOVE_V= 2, HALF_RESIZE_V= 4, HALF_RESIZE_H= 8,
			SHIFT= 0x30, SHIFT_LEFT= 0x10, SHIFT_RIGHT= 0x20, SHIFT_RESIZES= 0xc0
		};

		virtual void SetWindowResizingFlags(HWND hwnd, ResizeFlag resize_flag, int half_flags= 0) = 0;

		virtual ~CDlgResizer() {}
	};

	class CDialog : public CWnd
	{
	public:
		CDialog();
		CDialog(UINT resID, CWnd* parent = 0);
//		CDialog(UINT nResID, HWND hWndParent);
//		CDialog(LPCTSTR lpszResName, HWND hWndParent);
//		CDialog(LPCDLGTEMPLATE lpTemplate, HWND hWndParent);
		virtual ~CDialog();

		void Create(CWnd* parent, UINT res_id);

	//	virtual HWND Create(HWND hWndParent);
		virtual INT_PTR DoModal();
		//virtual HWND DoModeless();

		virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

		void SetCtrlResizer(boost::shared_ptr<CDlgResizer> resizer);
	protected:
		// These are the functions you might wish to override
		//virtual BOOL DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		//virtual BOOL DialogProcDefault(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		virtual void EndDialog(INT_PTR nResult);
		virtual void OnCancel();
		virtual BOOL OnInitDialog();
		virtual void OnOK();

		void OnClose();

		// Can't override this function
		static BOOL CALLBACK StaticDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	protected:
		BOOL HandleInitDialog(HWND focus_ctrl, void* param);
		void OnSize(UINT type, int cx, int cy);

	const TCHAR* dlg_template_name_;	// dlg name or MAKEINTRESOURCE
	HGLOBAL dlg_template_handle_;		// indirect (dlg_template_ == NULL)
	LPCDLGTEMPLATE dlg_template_;  // indirect if (dlg_template_name_ == NULL)
	void* m_lpDialogInit;           // DLGINIT resource data
	int dlg_result_;
	CWnd* parent_wnd_;

	private:
		//BOOL IsIndirect;				// a flag for Indirect dialogs
		//BOOL IsModal;					// a flag for modal dialogs
		//LPCTSTR m_lpszResName;			// the resource name for the dialog
		//LPCDLGTEMPLATE m_lpTemplate;	// the dialog template for indirect dialogs

		boost::shared_ptr<CDlgResizer> resizer_;
	protected:

//	BOOL CreateDlgIndirect(const DLGTEMPLATE* dialog_template, CWnd* parent_wnd, HINSTANCE instance);

		DECLARE_MESSAGE_MAP()
	};
/*
	inline CDialog::CDialog(LPCTSTR lpszResName, HWND hWndParent/ = NULL/)
		: IsIndirect(FALSE), IsModal(TRUE), m_lpszResName(lpszResName), m_lpTemplate(NULL)
	{
//		m_hWndParent = hWndParent;
		::InitCommonControls();
	}

	inline CDialog::CDialog(UINT nResID, HWND hWndParent/ = NULL/)
		: IsIndirect(FALSE), IsModal(TRUE), m_lpszResName(MAKEINTRESOURCE (nResID)), m_lpTemplate(NULL)
	{
//		m_hWndParent = hWndParent;
		::InitCommonControls();
	}

	//For indirect dialogs - created from a dialog box template in memory.
	inline CDialog::CDialog(LPCDLGTEMPLATE lpTemplate, HWND hWndParent/ = NULL/)
		: IsIndirect(TRUE), IsModal(TRUE), m_lpszResName(NULL), m_lpTemplate(lpTemplate)
	{
//		m_hWndParent = hWndParent;
		::InitCommonControls();
	}
*/

	//inline HWND CDialog::Create(HWND hWndParent = 0)
	//{
	//	// Allow a dialog to be used as a child window
	//	//SetParent(hWndParent);
	//	//return DoModeless();
	//	return 0;
	//}

	boost::shared_ptr<CDlgResizer> CreateSimpleDlgCtrlResizer(CWnd& dialog, CPoint center);

} // namespace mcr

#endif // DIALOG_H
