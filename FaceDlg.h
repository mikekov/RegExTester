// FaceDlg.h : header file
//
#pragma once

#include "ScintillaCtrl.h"

// FaceDlg dialog
class FaceDlg : public CDialog
{
// Construction
public:
	FaceDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_REGEX_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CScintillaCtrl regex_text_;
	CScintillaCtrl replacement_;
	CScintillaCtrl test_text_;
//	CStatic line_separator_;
	CScintillaCtrl result_;
	CScintillaCtrl status_;
	CCheckBox case_insensitive_;
	CCheckBox dot_matches_eol_;
	CCheckBox partial_matches_;
	CRadioButton regex_type_;
	CRadioButton show_results_;

	bool in_update_;
CDialog page_;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	void OnSize(UINT type, int cx, int cy);
	void OnRegExChanged();
	void OnClose();

	DECLARE_MESSAGE_MAP()

//	BOOL OnEraseBkgnd(CDC* dc);
};
