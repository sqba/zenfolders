// xmlpropd.cpp: implementation of the CFolderPropertiesDlg class.
//
//////////////////////////////////////////////////////////////////////

#include <crtdbg.h>
#include <stdio.h>
#include <tchar.h>
#include "shlfldr.h"
#include "util/settings.h"
#include "resource.h"
#include "viewlist.h"
#include "cfgxml.h"
#include "dlglist.h"


extern LPVIEWSLIST		g_pViewList;
extern HINSTANCE		g_hInst;
extern LPCONFIGXML		g_pConfigXML;
extern LPPIDLMGR		g_pPidlMgr;
extern LPDIALOGSLIST	g_pDialogList;


BOOL CALLBACK PropertiesDlgProc (HWND hDlg,
								 UINT message,
								 WPARAM wParam,
								 LPARAM lParam)
{
	CFolderPropertiesDlg *pDlg = NULL;

	switch (message)
	{
	case WM_INITDIALOG :
		pDlg = (CFolderPropertiesDlg*)lParam;
		return pDlg->OnInit(hDlg);
		
	case WM_COMMAND :
		pDlg = (CFolderPropertiesDlg*)::GetWindowLong(hDlg, DWL_USER);

		switch (LOWORD (wParam))
		{
		case IDOK:
			return pDlg->OnOk(hDlg);

		case IDCANCEL:
			return pDlg->OnCancel(hDlg);

		case IDAPPLY:
			return pDlg->OnApply(hDlg);
		}
		break ;

	case WM_CLOSE :
		pDlg = (CFolderPropertiesDlg*)lParam;
		pDlg->OnClose(hDlg);
		return FALSE ;
	}
	return FALSE ;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderPropertiesDlg::CFolderPropertiesDlg(CShellFolder *pParent,
										   LPCITEMIDLIST pidl)
{
	m_hDlg = NULL;
	m_pParent = pParent;

	if(NULL == pidl)
	{
		delete this;
		return;
	}

	m_pidl = pidl;
}

CFolderPropertiesDlg::~CFolderPropertiesDlg()
{
	_RPTF0(_CRT_WARN, "~CFolderPropertiesDlg\n");
	if(m_hDlg)
		::DestroyWindow(m_hDlg);
}





//////////////////////////////////////////////////////////////////////
// Event handlers

BOOL CFolderPropertiesDlg::OnInit(HWND hDlg)
{
	g_pDialogList->AddToList(this);

	CenterDialog(hDlg);

	// Treba videti zasto se pidl ne apdejtuje
	MSXML2::IXMLDOMNodePtr ptrNode = m_pidl.GetNode();
	LPPIDLDATA pData = m_pidl.GetData();
	CConfigXML::GetFolderInfo(pData, ptrNode);

	InitFolderName(hDlg);
	InitRanking(hDlg);
	InitQueryString(hDlg);
	InitMaxResults(hDlg);
	InitCategory(hDlg);

	return TRUE ;
}

BOOL CFolderPropertiesDlg::OnApply(HWND hDlg)
{
	return ApplyChanges(hDlg);
}

BOOL CFolderPropertiesDlg::OnOk(HWND hDlg)
{
	ApplyChanges(hDlg);
	EndDialog (hDlg, 0) ;
	return TRUE ;
}

BOOL CFolderPropertiesDlg::OnCancel(HWND hDlg)
{
	EndDialog (hDlg, 0) ;
	return TRUE ;
}

BOOL CFolderPropertiesDlg::OnClose(HWND hDlg)
{
	g_pDialogList->RemoveFromList(this);
	return TRUE ;
}












void CFolderPropertiesDlg::Show()
{
	m_hDlg = ::CreateDialogParam(
		g_hInst,
		MAKEINTRESOURCE(IDD_FOLDER_PROPERTIES), 
		NULL, //HWND hWndParent
		PropertiesDlgProc,
		(LPARAM)this);

	::SetWindowLong(m_hDlg, DWL_USER, (LONG)this);

	::ShowWindow(m_hDlg, SW_SHOW) ;

	::SetFocus( GetDlgItem(m_hDlg, IDC_FOLDER_NAME) );
}

void CFolderPropertiesDlg::CenterDialog(HWND hDlg)
{
	HWND hwndOwner; 
	RECT rc, rcDlg, rcOwner;

	if ((hwndOwner = GetParent(hDlg)) == NULL) 
		hwndOwner = GetDesktopWindow(); 

	GetWindowRect(hwndOwner, &rcOwner); 
	GetWindowRect(hDlg, &rcDlg); 
	CopyRect(&rc, &rcOwner); 

	// Offset the owner and dialog box rectangles so that right and bottom 
	// values represent the width and height, and then offset the owner again 
	// to discard space taken up by the dialog box. 

	OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
	OffsetRect(&rc, -rc.left, -rc.top); 
	OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

	// The new position is the sum of half the remaining space and the owner's 
	// original position. 

	::SetWindowPos(hDlg, 
				 HWND_TOP, 
				 rcOwner.left + (rc.right / 2), 
				 rcOwner.top + (rc.bottom / 2), 
				 0, 0,          // Ignores size arguments. 
				 SWP_NOSIZE); 

	/*if (GetDlgCtrlID((HWND) wParam) != IDC_FOLDER_NAME) 
	{ 
		SetFocus(GetDlgItem(hDlg, IDC_FOLDER_NAME)); 
		return FALSE; 
	}*/
}

void CFolderPropertiesDlg::InitFolderName(HWND hDlg)
{
	LPPIDLDATA pData = m_pidl.GetData();

	::SetDlgItemText(hDlg, IDC_FOLDER_NAME, pData->szName);

	SetCaption(hDlg, pData);

//	HWND hName = GetDlgItem(hDlg, IDC_FOLDER_NAME);
//	::SetFocus(hName);
}

void CFolderPropertiesDlg::InitQueryString(HWND hDlg)
{
	LPPIDLDATA pData = m_pidl.GetData();
	::SetDlgItemText(hDlg, IDC_FOLDER_QUERY, pData->folderData.szQuery);
}

void CFolderPropertiesDlg::InitMaxResults(HWND hDlg)
{
	LPPIDLDATA pData = m_pidl.GetData();
	::SetDlgItemInt(hDlg, IDC_MAXRESULTS, pData->folderData.maxResults, FALSE);
}

void CFolderPropertiesDlg::InitRanking(HWND hDlg)
{
	UINT radioButton = IDC_RANKING_RELEVANCE;

	LPPIDLDATA pData = m_pidl.GetData();

	if(pData->folderData.ranking == RECENCY_RANKING)
		radioButton = IDC_RANKING_RECENCY;

	::CheckRadioButton(
		hDlg,
		IDC_RANKING_RELEVANCE,
		IDC_RANKING_RECENCY,
		radioButton) ;
}

void CFolderPropertiesDlg::InitCategory(HWND hDlg)
{
	HWND hCombo = GetDlgItem(hDlg, IDC_CATEGORY);

	SendMessage(hCombo, CB_LIMITTEXT , 20, 0);  

	LPCTSTR categories[] = 
	{
		"email", "web", "im", "file", "contact",
		"calendar", "task", "note", "journal"
	};
	int n = sizeof(categories) / sizeof(LPCTSTR);
	for(int i=0; i<n; i++)
	{
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)categories[i]);
	}

	int selIndex = 3; // file

	LPPIDLDATA pData = m_pidl.GetData();

	int index = SendMessage(hCombo, CB_FINDSTRING, -1, (LPARAM)pData->folderData.szCategory);
	if(CB_ERR != index)
		selIndex = index;

	SendMessage(hCombo, CB_SETCURSEL , selIndex, 0); 
}







BOOL CFolderPropertiesDlg::ApplyChanges(HWND hDlg)
{
	SetCursor( LoadCursor(NULL, IDC_WAIT) );

	BOOL bSave = FALSE;
	BOOL bResult = FALSE;
	BOOL bName = FALSE;

	PIDLDATA dataNew;
	LPPIDLDATA pDataOld = m_pidl.GetData();
	memcpy(&dataNew, pDataOld, sizeof(PIDLDATA));

	if( SetMaxResults(hDlg, &dataNew) )
		bSave = TRUE;

	if( SetRanking(hDlg, &dataNew) )
		bSave = TRUE;

	if( SetCategory(hDlg, &dataNew) )
		bSave = TRUE;

	if( SetQueryString(hDlg, &dataNew) )
		bSave = TRUE;

	if( SetFolderName(hDlg, &dataNew) )
	{
		bName = bSave = TRUE;
	}

	if(bSave)
	{
		CPidl pidlNew( m_pidl.GetRelative() );
		LPPIDLDATA pDataTmp = pidlNew.GetData();
		memcpy(pDataTmp, &dataNew, sizeof(PIDLDATA));
		CPidl pidlFQNew = m_pParent->CreateFQPidl( pidlNew.GetRelative() );
		if( g_pConfigXML->SaveFolder(m_pidl.GetFull(), pidlFQNew.GetFull()) )
		{
			TRACE_PIDL_PATH("CFolderPropertiesDlg::ApplyChanges pidlFQOld: %s\n", &m_pidl);
			TRACE_PIDL_PATH("CFolderPropertiesDlg::ApplyChanges pidlFQNew: %s\n", &pidlFQNew);

			if(bName)
			{
				::SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_IDLIST, m_pidl.GetFull(), pidlFQNew.GetFull());
				SetCaption(m_hDlg, &dataNew);
			}
			else
				::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlFQNew.GetFull(), NULL);

			g_pViewList->Refresh();
			bResult = TRUE;
		}
	}

	SetCursor( LoadCursor(NULL, IDC_ARROW) );

	return bResult;
}

BOOL CFolderPropertiesDlg::SetFolderName(HWND hDlg, LPPIDLDATA pDataNew)
{
	UINT uLen;
	BOOL bSave = FALSE;
	TCHAR szNewName[40] = {0};
//	TCHAR attribName[] = _T("name");

	uLen = ::GetDlgItemText(
		hDlg,
		IDC_FOLDER_NAME,
		szNewName,
		sizeof(szNewName)/sizeof(TCHAR));

	LPPIDLDATA pData = m_pidl.GetData();

	if(uLen > 0)
	{
		if(0 != lstrcmp(pData->szName, szNewName))
		{
			lstrcpyn(pDataNew->szName, szNewName, sizeof(pData->szName)/sizeof(TCHAR));
//			m_pParent->Rename(szNewName);
			bSave = TRUE;
		}
	}

	return bSave;
}

BOOL CFolderPropertiesDlg::SetQueryString(HWND hDlg, LPPIDLDATA pDataNew)
{
	UINT uLen;
	BOOL bSave = FALSE;
	TCHAR szNewQuery[200] = {0};
	TCHAR attribName[] = _T("query");

	uLen = ::GetDlgItemText(
		hDlg,
		IDC_FOLDER_QUERY,
		szNewQuery,
		sizeof(szNewQuery)/sizeof(TCHAR));

	LPPIDLDATA pData = m_pidl.GetData();

	if(0 != lstrcmp(pData->folderData.szQuery, szNewQuery))
	{
		int len = sizeof(pData->folderData.szQuery)/sizeof(TCHAR);
		lstrcpyn(pDataNew->folderData.szQuery, szNewQuery, len);
		bSave = TRUE;
	}

	return bSave;
}

BOOL CFolderPropertiesDlg::SetMaxResults(HWND hDlg, LPPIDLDATA pDataNew)
{
	UINT maxResults;
	BOOL bTranslated;
	BOOL bSave = FALSE;
	TCHAR attribName[] = _T("maxResults");

	maxResults = ::GetDlgItemInt(hDlg, IDC_MAXRESULTS, &bTranslated, FALSE);

	if(!bTranslated)
		maxResults = 0;

	LPPIDLDATA pData = m_pidl.GetData();

	if(maxResults != pData->folderData.maxResults)
	{
		pDataNew->folderData.maxResults = maxResults;
		bSave = TRUE;
	}

	return bSave;
}

BOOL CFolderPropertiesDlg::SetRanking(HWND hDlg, LPPIDLDATA pDataNew)
{
	BOOL bSave = FALSE;
	UINT ranking = RELEVANCE_RANKING;

	if( !IsDlgButtonChecked(hDlg, IDC_RANKING_RELEVANCE) )
		ranking = RECENCY_RANKING;

	LPPIDLDATA pData = m_pidl.GetData();
	if(pData->folderData.ranking != ranking)
	{
		pDataNew->folderData.ranking = ranking;
		bSave = TRUE;
	}

	return bSave;
}

BOOL CFolderPropertiesDlg::SetCategory(HWND hDlg, LPPIDLDATA pDataNew)
{
	BOOL bSave = FALSE;
	TCHAR szCategory[21] = {0};

	HWND hCombo = GetDlgItem(hDlg, IDC_CATEGORY);

	//int index = SendMessage(hCombo, CB_GETCURSEL , 0, 0);
	//SendMessage(hCombo, CB_GETLBTEXT , index, (LPARAM)szCategory);
	::GetWindowText(hCombo, szCategory, 20);

	UINT uLen = lstrlen(szCategory);

	LPPIDLDATA pData = m_pidl.GetData();
	if(0 != lstrcmp(pData->folderData.szCategory, szCategory))
	{
		int len = sizeof(pData->folderData.szCategory)/sizeof(TCHAR);
		lstrcpyn(pDataNew->folderData.szCategory, szCategory, len);
		bSave = TRUE;
	}

	return bSave;
}







void CFolderPropertiesDlg::SetCaption(HWND hDlg, LPPIDLDATA pidl)
{
	TCHAR szCaption[100] = {0};
	sprintf(szCaption, "%s Properties", pidl->szName);
	::SetWindowText(hDlg, szCaption);
}
