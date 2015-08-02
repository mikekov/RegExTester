#pragma once
#include "wincore.h"
#include "signals.h"

namespace mcr {


class CButton : public CWnd
{
public:
	CButton();

//	BOOL Create(DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);
//	BOOL CreateEx(DWORD ex_style, DWORD style, const RECT& rect, CWnd* parent_wnd, UINT id);

	//int GetItemCount() const;

	//int GetCurSel() const;
	//int SetCurSel(int item);

};


class CCheckBox : public CWnd
{
public:
	CCheckBox();

	bool GetChecked() const;
	void SetChecked(bool checked);

	// connect event
	slot_connection OnChecked(Signal::fn callback);

protected:
	virtual void PreSubclassWindow();
	virtual BOOL OnChildNotify(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result);

private:
	mutable bool checked_;
	Signal sig_checked_;
};


class CRadioButton : public CWnd
{
public:
	CRadioButton();

	int GetSelected() const;
	void SetSelected(int selected);

	// connect event
	slot_connection OnSelected(Signal::fn callback);

protected:
	virtual void PreSubclassWindow();
	virtual BOOL OnChildNotify(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result);

private:
	mutable int selected_;	// 0-based index of selected radio button
	Signal sig_selected_;
};


} // namespace
