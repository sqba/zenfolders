#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <windows.h>
#include <shlobj.h>
#include "listview.h"

#define IDM_VIEW_IDW	(FCIDM_SHVIEWFIRST + 0x501)

class CToolBar  
{
public:
	static BOOL OnTtnNeedTextA(LPNMHDR);
	static BOOL OnTtnNeedTextW(LPNMHDR);
	static VOID MergeToolbar(LPSHELLBROWSER);
	static VOID OnToolbarDropdown(CListView*, LPSHELLBROWSER, HWND, HWND, LPARAM);

//	static void MergeFileMenu(HMENU);
//	static void MergeViewMenu(HMENU);
//	BOOL OnCommand(DWORD);
};

#endif   //TOOLBAR_H
