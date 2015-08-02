#pragma once


enum	// constants
{
	ID_HELP =                         0xE146,	// first attempt for F1
	AFX_IDW_PANE_FIRST =              0xE900,	// first pane (256 max)
	AFX_IDW_PANE_LAST =               0xE9ff,
	AFX_IDW_REBAR =                   0xE804,	// rebar
	AFX_IDW_STATUS_BAR =              0xE801,	// status bar
	AFX_IDW_TOOLBAR =                 0xE800	// main toolbar
};

enum	// extra messages
{
	WM_KICKIDLE=         0x036A,	// (params unused) causes idles to kick in
	WM_IDLEUPDATECMDUI=  0x0363		// wParam == disable if no handler
};
