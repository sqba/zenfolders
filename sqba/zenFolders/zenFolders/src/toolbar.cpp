// toolbar.cpp: implementation of the CToolBar class.
//
//////////////////////////////////////////////////////////////////////

#include "toolbar.h"
#include "commands.h"
#include "resource.h"




extern HINSTANCE	g_hInst;


typedef struct
{
    HWND        hwndFrom;
    VARIANTARG  *pva;
    DWORD       dwUnused;
} TBDDDATA, *LPTBDDDATA;


typedef struct
{
	UINT  idCommand;
	UINT  idString;
	UINT  uImageSet;
	UINT  iImage;
	BYTE  bState;
	BYTE  bStyle;
} MYTOOLINFO, *LPMYTOOLINFO;


MYTOOLINFO g_Tools[] = 
{
	{
		// New folder
		IDM_CREATE_FOLDER, IDS_CREATE_FOLDER,
		IDB_VIEW_SMALL_COLOR, VIEW_NEWFOLDER,
		TBSTATE_ENABLED, TBSTYLE_BUTTON
	},
	{
		// Delete folder
		IDM_DELETE_FOLDER, IDS_DELETE_FOLDER,
		IDB_STD_SMALL_COLOR, STD_DELETE,
		TBSTATE_ENABLED, TBSTYLE_BUTTON
	},
	{
		// File/folder properties
		IDM_PROPERTIES, IDS_PROPERTIES,
		IDB_STD_SMALL_COLOR, STD_PROPERTIES,
		TBSTATE_ENABLED, TBSTYLE_BUTTON
	},
	{
		// View style
		IDM_VIEW_IDW, IDS_MI_VIEW_IDW,
		IDB_VIEW_SMALL_COLOR, VIEW_LIST,
		TBSTATE_ENABLED, TBSTYLE_DROPDOWN
	},
	{-1, 0, 0, 0, 0, 0},
};





BOOL CToolBar::OnTtnNeedTextA(LPNMHDR lpnmh)
{
	LPNMTTDISPINFOA pttt = (LPNMTTDISPINFOA)lpnmh;
	
	for(int i=0; -1 != g_Tools[i].idCommand; i++)
	{
		if(g_Tools[i].idCommand == pttt->hdr.idFrom)
		{
			::LoadStringA(
				g_hInst,
				g_Tools[i].idString,
				pttt->szText,
				sizeof(pttt->szText));
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CToolBar::OnTtnNeedTextW(LPNMHDR lpnmh)
{
	LPNMTTDISPINFOW pttt = (LPNMTTDISPINFOW)lpnmh;
	
	for(int i=0; -1 != g_Tools[i].idCommand; i++)
	{
		if(g_Tools[i].idCommand == pttt->hdr.idFrom)
		{
			::LoadStringW(
				g_hInst,
				g_Tools[i].idString,
				pttt->szText,
				sizeof(pttt->szText));
			return TRUE;
		}
	}

	return FALSE;
}

VOID CToolBar::MergeToolbar(LPSHELLBROWSER pShellBrowser)
{
	int         i;
	TBADDBITMAP tbab;
	LRESULT     lStdOffset;
	LRESULT     lViewOffset;
	
	pShellBrowser->SetToolbarItems(NULL, 0, FCT_MERGE);
	
	tbab.hInst = HINST_COMMCTRL;
	tbab.nID = (int)IDB_STD_SMALL_COLOR;
	pShellBrowser->SendControlMsg(FCW_TOOLBAR, TB_ADDBITMAP, 0, (LPARAM)&tbab, &lStdOffset);
	
	tbab.hInst = HINST_COMMCTRL;
	tbab.nID = (int)IDB_VIEW_SMALL_COLOR;
	pShellBrowser->SendControlMsg(FCW_TOOLBAR, TB_ADDBITMAP, 0, (LPARAM)&tbab, &lViewOffset);
	
	//get the number of items in tool array
	for(i = 0; -1 != g_Tools[i].idCommand; i++) {}
	
	LPTBBUTTON  ptbb = (LPTBBUTTON)::GlobalAlloc(GPTR, sizeof(TBBUTTON) * i);
	
	if(ptbb)
	{
		for(i = 0; -1 != g_Tools[i].idCommand; i++)
		{
			(ptbb + i)->iBitmap = 0;
			switch(g_Tools[i].uImageSet)
			{
			case IDB_STD_SMALL_COLOR:
				(ptbb + i)->iBitmap = lStdOffset + g_Tools[i].iImage;
				break;
				
			case IDB_VIEW_SMALL_COLOR:
				(ptbb + i)->iBitmap = lViewOffset + g_Tools[i].iImage;
				break;
			}
			
			(ptbb + i)->idCommand = g_Tools[i].idCommand;
			(ptbb + i)->fsState = g_Tools[i].bState;
			(ptbb + i)->fsStyle = g_Tools[i].bStyle;
			(ptbb + i)->dwData = 0;
			(ptbb + i)->iString = 0;

			//if(((g_Tools[i].idCommand == IDM_PROPERTIES)
			//	|| (g_Tools[i].idCommand == IDM_DELETE_FOLDER))
			/*if((g_Tools[i].idCommand == IDM_DELETE_FOLDER) && pSFParent->IsRoot())
			{
				(ptbb + i)->fsState = TBSTATE_INDETERMINATE;
			}*/
		}
		
		pShellBrowser->SetToolbarItems(ptbb, i, FCT_MERGE);
		
		::GlobalFree((HGLOBAL)ptbb);
	}

	//::SendMessage(?, TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_DRAWDDARROWS);
}

VOID CToolBar::OnToolbarDropdown(CListView *pListView,
								 LPSHELLBROWSER pShellBrowser,
								 HWND hwndParent,
								 HWND hWndView,
								 LPARAM lParam)
{
	LPTBDDDATA ptbd = (LPTBDDDATA)lParam;
	if(ptbd->pva && (VT_INT_PTR == ptbd->pva->vt))
	{
		HMENU hMenu  = ::LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_POPUP)); 
		HMENU hPopupMenu = ::GetSubMenu(hMenu, 0);
		if(hMenu)
		{
			LPRECT prc = (LPRECT)ptbd->pva->byref;

			if(pListView)
			{
				LONG style = pListView->GetStyle();
				int item = 0;
				switch(style)
				{
				case LVS_ICON:
					item = IDC_LARGE_ICONS;
					break;
				case LVS_SMALLICON:
					item = IDC_SMALL_ICONS;
					break;
				case LVS_LIST:
					item = IDC_LIST;
					break;
				case LVS_REPORT:
					item = IDC_DETAILS;
					break;
				}
				::CheckMenuItem(hPopupMenu, item, MF_BYCOMMAND | MF_CHECKED);
			}

			::TrackPopupMenu(
				hPopupMenu,
				TPM_LEFTALIGN,
				prc->left,
				prc->bottom,
				0,
				hWndView,
				NULL);
			::DestroyMenu(hMenu);
		}
	} 
}

/*
void CToolBar::MergeFileMenu(HMENU hSubMenu)
{
	if(!hSubMenu)
		return;

	MENUITEMINFO   mii = {0};
	TCHAR          szText[MAX_PATH];
	
	//ZeroMemory(&mii, sizeof(mii));
	
	//add a separator
	mii.fMask	= MIIM_TYPE | MIIM_ID | MIIM_STATE;
	mii.fType	= MFT_SEPARATOR;
	mii.fState	= MFS_ENABLED;
	
	//insert this item at the beginning of the menu
	::InsertMenuItem(hSubMenu, 0, TRUE, &mii);
	
	//add the file menu items
	::LoadString(g_hInst, IDS_MI_FILEITEM, szText, sizeof(szText));
	mii.cbSize		= sizeof(mii);
	mii.fMask		= MIIM_TYPE | MIIM_ID | MIIM_STATE;
	mii.fType		= MFT_STRING;
	mii.fState		= MFS_ENABLED;
	mii.dwTypeData	= szText;
	mii.wID			= IDM_MYFILEITEM;
	
	//insert this item at the beginning of the menu
	::InsertMenuItem(hSubMenu, 0, TRUE, &mii);
}

void CToolBar::MergeViewMenu(HMENU hSubMenu)
{
	if(!hSubMenu)
		return;

	MENUITEMINFO   mii = {0};
	TCHAR          szText[MAX_PATH];
	
	//ZeroMemory(&mii, sizeof(mii));
	
	//add a separator at the correct position in the menu
	mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
	mii.fType = MFT_SEPARATOR;
	mii.fState = MFS_ENABLED;
	InsertMenuItem(hSubMenu, FCIDM_MENU_VIEW_SEP_OPTIONS, FALSE, &mii);
	
	//add the view menu items at the correct position in the menu
	LoadString(g_hInst, IDS_MI_VIEW_KEYS, szText, sizeof(szText));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.dwTypeData = szText;
	mii.wID = IDM_VIEW_KEYS;
	InsertMenuItem(hSubMenu, FCIDM_MENU_VIEW_SEP_OPTIONS, FALSE, &mii);
}

HMENU CToolBar::BuildMenu(void)
{
	HMENU hSubMenu = ::CreatePopupMenu();
	
	if(hSubMenu)
	{
		TCHAR          szText[MAX_PATH];
		MENUITEMINFO   mii;
		int            nTools,
			i;
		//get the number of items in our global array
		for(nTools = 0; g_Tools[nTools].idCommand != -1; nTools++) {}
		
		//add the menu items
		for(i = 0; i < nTools; i++)
		{
			LoadString(
				g_hInst,
				g_Tools[i].idString,
				szText,
				sizeof(szText));
			
			ZeroMemory(&mii, sizeof(mii));
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
			
			if(TBSTYLE_SEP != g_Tools[i].bStyle)
			{
				mii.fType = MFT_STRING;
				mii.fState = MFS_ENABLED;
				mii.dwTypeData = szText;
				mii.wID = g_Tools[i].idCommand;
			}
			else
			{
				mii.fType = MFT_SEPARATOR;
			}
			
			//tack this item onto the end of the menu
			InsertMenuItem(hSubMenu, (UINT)-1, TRUE, &mii);
		}
	}
	
	return hSubMenu;
}

BOOL CToolBar::OnCommand(DWORD dwCmdID)
{
	if(IDM_VIEW_IDW == dwCmdID)
	{
		OnToolbarDropdown();
		return TRUE;
	}
	return FALSE;
}
*/
