#pragma once
#include "cmdtarget.h"

namespace mcr {


class CWndThread : public CCmdTarget
{
public:
	CWndThread();
	virtual ~CWndThread();

	virtual BOOL InitInstance();

	virtual int ExitInstance();

};

}
