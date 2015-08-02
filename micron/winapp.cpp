#include "stdafx.h"
#include "winapp.h"
#include "micron.h"

namespace mcr {

BEGIN_MESSAGE_MAP(CWinApp, CWinThread)
END_MESSAGE_MAP()

//CWinApp* g_the_app= 0;
//static HINSTANCE app_instance_;
//static HINSTANCE rsc_instance_;


//CWinApp::CWinApp()
//{
//	m_pMainWnd = 0;
//	m_hInstance = 0;
//	rsc_instance_ = 0;
//	app_instance_ = 0;
//
//	g_the_app = this;
//}


//BOOL CWinApp::InitInstance()
//{
//	return false;
//}


void CWinApp::SetRegistryKey(const TCHAR* section)
{
}


HICON CWinApp::LoadIcon(UINT id)
{
	return ::LoadIcon(mcr::AppGetResourceHandle(), MAKEINTRESOURCE(id));
}


HCURSOR CWinApp::LoadStandardCursor(UINT id)
{
	return ::LoadCursor(0, MAKEINTRESOURCE(id));
}


HCURSOR CWinApp::LoadStandardCursor(const TCHAR* id)
{
	return ::LoadCursor(0, id);
}


void CWinApp::OnHelp()
{
}

	//CWnd* ;
	//HINSTANCE m_hInstance;

//extern CApp theApp;


CWinApp* AfxGetApp()
{
	return mcr::GetApp();
}

} // namespace


INT WINAPI _tWinMain(HINSTANCE instance, HINSTANCE, LPTSTR, int)
{
	if (AfxGetApp() == 0)
		return -1;

	AfxGetApp()->m_hInstance = instance;
	//app_instance_ = rsc_instance_ = instance;

	// Start Win32++
//	CDialogApp theApp;

	if (AfxGetApp()->InitInstance())
	{
		// run
		//
	}

	// Run the application
//	return theApp.Run();

	return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace mcr {


CWinApp* g_app= 0;


CWinApp::CWinApp()
{
	g_app = this;
	m_pMainWnd = 0;
	m_hInstance = 0;
	//rsc_instance_ = 0;
	//app_instance_ = 0;

	m_hInstance = (HINSTANCE) ::GetModuleHandle(0);
	m_hResource = m_hInstance;
}

CWinApp* GetApp()
{
	return g_app;
}


CWinApp::~CWinApp()
{}


BOOL CWinApp::InitInstance()
{
	return true;
}


int CWinApp::MessageLoop()
{
	return 0;
}


int CWinApp::Run()
{
	return 0;
}


} // namespace
