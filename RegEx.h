// Slownik.h : main header file for the PROJECT_NAME application
//

#pragma once

#include "micron/micron.h"

#ifndef __AFXWIN_H__
	//#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CApp:
// See Slownik.cpp for the implementation of this class
//

class CApp : public CWinApp
{
public:
	CApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CApp theApp;