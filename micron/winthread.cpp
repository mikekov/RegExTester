#include "stdafx.h"
#include "micron.h"
//#include "winthread.h"

namespace mcr {


BEGIN_MESSAGE_MAP(CWinThread, CCmdTarget)
END_MESSAGE_MAP()


CWndThread::CWndThread()
{}

CWndThread::~CWndThread()
{}


BOOL CWndThread::InitInstance()
{
	return true;
}


int CWndThread::ExitInstance()
{
	//TODO: save settings?

	//TODO: ret code

	return 0;
}

} // namespace
