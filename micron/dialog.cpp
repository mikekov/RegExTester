#include "stdafx.h"
#include "micron.h"

BEGIN_MESSAGE_MAP(CDialog, CWnd)
	ON_WM_INITDIALOG()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_COMMAND(IDCANCEL, &OnCancel)
END_MESSAGE_MAP()


void CDialog::DoDataExchange(CDataExchange* pDX) // DDX/DDV support
{
}

CDialog::CDialog()
{
	dlg_result_ = 0;
	dlg_template_name_ = 0;
	dlg_template_handle_ = 0;
	dlg_template_ = 0;
	m_lpDialogInit = 0;
	parent_wnd_ = 0;
}

CDialog::CDialog(UINT rsc_id, CWnd* parent)
{
	dlg_result_ = 0;
	dlg_template_name_ = MAKEINTRESOURCE(rsc_id);
	dlg_template_handle_ = 0;
	dlg_template_ = 0;
	m_lpDialogInit = 0;
	parent_wnd_ = parent;
}

CDialog::~CDialog()
{
	//if (handle() != NULL)
	//{
	//	if (IsModal)
	//		::EndDialog(handle(), 0);
	//	else
	//		Destroy();
	//}
}


BOOL CDialog::HandleInitDialog(HWND focus_ctrl, void* param)
{
	LRESULT result= Default();

	//TODO: init dlg with dlg data

	if (!UpdateData(false))
	{
//		ASSERT(false, "UpdateData failed during dialog init.");
		EndDialog(-1);
		return false;
	}

	return OnInitDialog();
}


BOOL CDialog::OnInitDialog()
{
	return true;
}


//INT_PTR CDialog::DoModal()
//{
//	return false;
//}

//BOOL CDialog::CreateDlgIndirect(const DLGTEMPLATE* dialog_template, CWnd* parent_wnd, HINSTANCE instance)
//{
//}

struct DialogTemplateHelper
{
	DialogTemplateHelper(const DLGTEMPLATE* dlg_template, HGLOBAL dlg_template_handle, const TCHAR* dlg_template_name)
	{
		instance_ = mcr::AppGetResourceHandle();
		dialog_template_ = dlg_template;
		dialog_template_handle_ = dlg_template_handle;
		free_handle_ = false;

		if (dlg_template_name != 0)
		{
			instance_ = mcr::AppFindResourceHandle(dlg_template_name, RT_DIALOG);
			HRSRC resource= ::FindResource(instance_, dlg_template_name, RT_DIALOG);
			dialog_template_handle_ = LoadResource(instance_, resource);
			free_handle_ = dialog_template_handle_ != 0;
		}

		if (dialog_template_handle_ != NULL)
			dialog_template_ = reinterpret_cast<const DLGTEMPLATE*>(::LockResource(dialog_template_handle_));
	}

	~DialogTemplateHelper()
	{
		if (free_handle_!= NULL)
			FreeResource(dialog_template_handle_);
	}

	const DLGTEMPLATE* get_template() const
	{
		return dialog_template_;
	}

	HINSTANCE get_instance() const
	{
		return instance_;
	}

private:
	const DLGTEMPLATE* dialog_template_;
	bool free_handle_;
	HGLOBAL dialog_template_handle_;
	HINSTANCE instance_;
};


struct ParentWndHelper
{
	ParentWndHelper(CDialog& dlg, HWND parent) : dlg_(dlg), parent_(parent)
	{
		reenable_ = false;
		if (parent && parent != ::GetDesktopWindow() && ::IsWindowEnabled(parent))
		{
			::EnableWindow(parent, false);
			reenable_ = true;
		}
	}

	~ParentWndHelper()
	{
		if (!parent_)
			return;

		if (reenable_)
			::EnableWindow(parent_, true);

		if (dlg_.handle())
			dlg_.HideWnd();

		if (dlg_.handle() == ::GetActiveWindow())
			::SetActiveWindow(parent_);
	}

private:
	CDialog& dlg_;
	HWND parent_;
	bool reenable_;
};


INT_PTR CDialog::DoModal()
{
	ASSERT(dlg_template_name_ != NULL || dlg_template_handle_ != NULL || dlg_template_ != NULL);

	DialogTemplateHelper dlg(dlg_template_, dlg_template_handle_, dlg_template_name_);

	if (dlg.get_template() == 0)
		return -1;

	HWND parent= parent_wnd_->GetSafeHwnd();  //PreModal();

	{
		// disable parent window
		ParentWndHelper block(*this, parent);

		// turn on modal loop flag
		SetContinueModal();

		// create modeless dialog

		if (CreateDlgIndirect(dlg.get_template(), CWnd::FromHandle(parent), dlg.get_instance()))
		{
			// if init dialog finished without calling EndDialog, enter modal loop
			if (ContinueModal())
			{
				// enter modal loop
				//DWORD dwFlags = MLF_SHOWONIDLE;
				//if (GetStyle() & DS_NOIDLEMSG)
				//	dwFlags |= MLF_NOIDLEMSG;

				dlg_result_ = RunModalLoop();
			}
			//if (handle())
			//	HideWnd();
		}
	}

	// destroy modal window
	DestroyWindow();
//	PostModal();

	return dlg_result_;
}


void CDialog::Create(CWnd* parent_wnd, UINT rsc_id)
{
	dlg_result_ = 0;
	dlg_template_name_ = MAKEINTRESOURCE(rsc_id);
	dlg_template_handle_ = 0;
	dlg_template_ = 0;
	m_lpDialogInit = 0;
	parent_wnd_ = parent_wnd;

	ASSERT(dlg_template_name_ != NULL || dlg_template_handle_ != NULL || dlg_template_ != NULL);

	DialogTemplateHelper dlg(dlg_template_, dlg_template_handle_, dlg_template_name_);

	if (dlg.get_template() == 0)
		return;

	HWND parent= parent_wnd_->GetSafeHwnd();  //PreModal();

//	{
		// disable parent window
//		ParentWndHelper block(*this, parent);

		CreateDlgIndirect(dlg.get_template(), CWnd::FromHandle(parent), dlg.get_instance());

}

//void CDialog::EndDialog(int result)
//{
//}


BOOL CDialog::PreTranslateMessage(MSG* msg)
{
	// for modeless processing (or modal)
	ASSERT(handle() != 0);

	// allow tooltip messages to be filtered
	if (CWnd::PreTranslateMessage(msg))
		return true;

	// don't translate dialog messages when in Shift+F1 help mode
//	CFrameWnd* pFrameWnd = GetTopLevelFrame();
//	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
//		return FALSE;

	// fix around for VK_ESCAPE in a multiline Edit that is on a Dialog
	// that doesn't have a cancel or the cancel is disabled.
	//if (pMsg->message == WM_KEYDOWN &&
	//	(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CANCEL) &&
	//	(::GetWindowLong(pMsg->hwnd, GWL_STYLE) & ES_MULTILINE) &&
	//	_AfxCompareClassName(pMsg->hwnd, _T("Edit")))
	//{
	//	HWND hItem = ::GetDlgItem(m_hWnd, IDCANCEL);
	//	if (hItem == NULL || ::IsWindowEnabled(hItem))
	//	{
	//		SendMessage(WM_COMMAND, IDCANCEL, 0);
	//		return TRUE;
	//	}
	//}

	// filter both messages to dialog and from children
	return PreTranslateInput(msg);
}


void DDX_Control(CDataExchange* dx, int id, CWnd& control)
{
	if (control.handle())
		return;

//	ASSERT(!dx->save_and_validate_);

	dx->PrepareCtrl(id);

	if (!control.SubclassDlgItem(id, dx->dlg()))
		throw mcr::micron_exception("cannot subclass ctrl");
}


//CDataExchange::CDataExchange()
//{
//	dlg_ = 0;
//}


CWnd* CDataExchange::dlg() const
{
	return dlg_;
}


void CDialog::EndDialog(INT_PTR result)
{
	if (::IsWindow(handle()))
	{
	//	::EndDialog(handle(), nResult);
	//	else
	//		Destroy();
		EndModalLoop(result);
	}
}


void CDialog::OnCancel()
{

	EndDialog(IDCANCEL);
}


void CDialog::OnOK()
{
	//UpdateData

	EndDialog(IDOK);
}


void CDialog::OnClose()
{
//	CWnd::OnClose();
	EndModalLoop(IDCANCEL);
//	EndDialog(IDCANCEL);
}


void CDialog::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	if (type != SIZE_MINIMIZED && resizer_.get())
		resizer_->ResizeControls(*this, cx, cy);
}


void CDialog::SetCtrlResizer(boost::shared_ptr<mcr::CDlgResizer> resizer)
{
	resizer_ = resizer;
}
