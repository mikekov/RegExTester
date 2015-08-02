#pragma once
#include "wincore.h"


namespace mcr {


class CWinApp : public CWinThread
{
public:
	CWinApp();
	virtual ~CWinApp();

	// These are the functions you might wish to override
	virtual BOOL InitInstance();
	virtual int  MessageLoop();

	// These functions aren't intended to be overridden
	HINSTANCE GetInstanceHandle() const {return m_hInstance;}
	HINSTANCE GetResourceHandle() const {return (m_hResource ? m_hResource : m_hInstance);}
	int Run();
	void SetResourceHandle(HINSTANCE hResource) {m_hResource = hResource;}

	HICON LoadIcon(UINT id);
	void OnHelp();
	void SetRegistryKey(const TCHAR* section);
	HCURSOR LoadStandardCursor(UINT id);
	HCURSOR LoadStandardCursor(const TCHAR* id);
	HCURSOR LoadCursor(UINT id);

private:
	CWinApp(const CWinApp&);
	CWinApp& operator = (const CWinApp&);

	HINSTANCE m_hResource;		// handle to the applications resources

	//	HINSTANCE rsc_instance_;
	//	HINSTANCE app_instance_;

	DECLARE_MESSAGE_MAP()
public:
	HINSTANCE m_hInstance;		// handle to the applications instance
	CWnd* m_pMainWnd;
};


} // namespace
