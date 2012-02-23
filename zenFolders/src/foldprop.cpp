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


BOOL CALLBACK CFolderPropertiesDlg::DlgProc(HWND hDlg,
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
		pDlg = (CFolderPropertiesDlg*)::GetWindowLong(hDlg, DWL_USER);
		pDlg->OnClose(hDlg);
		return FALSE;

	/*case WM_DESTROY:
		pDlg = (CFolderPropertiesDlg*)::GetWindowLong(hDlg, DWL_USER);
		pDlg->OnDestroy(hDlg);
		return FALSE;*/
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderPropertiesDlg::CFolderPropertiesDlg(CShellFolder *pParent,
										   LPCITEMIDLIST pidl)
{
	if(NULL == pidl)
	{
		delete this;
		return;
	}

	m_hDlg		= NULL;
	m_pParent	= pParent;
	m_pidl		= pidl;
}

CFolderPropertiesDlg::CFolderPropertiesDlg(CShellFolder *pParent)
{
	m_hDlg		= NULL;
	m_pParent	= pParent;
	m_pidl		= NULL;
}

CFolderPropertiesDlg::~CFolderPropertiesDlg()
{
	_RPTF0(_CRT_WARN, "~CFolderPropertiesDlg\n");
	if(m_hDlg)
		::DestroyWindow(m_hDlg);
}





//////////////////////////////////////////////////////////////////////
// Event handlers

bool CFolderPropertiesDlg::OnInit(HWND hDlg)
{
	if( !g_pDialogList->AddToList(this) )
	{
		::DestroyWindow(hDlg);
		return false;
	}

	// Set dialog icon, big and small
	::SendMessage(hDlg, WM_SETICON, ICON_BIG,
		(LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ZENFOLDERS)));
	::SendMessage(hDlg, WM_SETICON, ICON_SMALL,
		(LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ZENFOLDERS)));

	if(NULL == (LPITEMIDLIST)m_pidl)
	{
		DisplayVersion(hDlg);
		DisplayPath(hDlg);
		return true;
	}

//	CenterDialog(hDlg);

	// Treba videti zasto se pidl ne apdejtuje
	MSXML2::IXMLDOMNodePtr ptrNode = m_pidl.GetNode();
	LPPIDLDATA pData = m_pidl.GetData();
	CConfigXML::GetFolderInfo(pData, ptrNode);

	InitFolderName(hDlg);
	InitRanking(hDlg);
	InitQueryString(hDlg);
	InitMaxResults(hDlg);
	InitCategory(hDlg);

	return true;
}

bool CFolderPropertiesDlg::OnApply(HWND hDlg)
{
	return ApplyChanges(hDlg);
}

bool CFolderPropertiesDlg::OnOk(HWND hDlg)
{
	g_pDialogList->RemoveFromList(this);
	ApplyChanges(hDlg);
	::EndDialog(hDlg, 0);
	return true ;
}

bool CFolderPropertiesDlg::OnCancel(HWND hDlg)
{
	g_pDialogList->RemoveFromList(this);
	::EndDialog(hDlg, 0);
	return true ;
}

bool CFolderPropertiesDlg::OnClose(HWND hDlg)
{
	g_pDialogList->RemoveFromList(this);
	return true;
}

bool CFolderPropertiesDlg::OnDestroy(HWND hDlg)
{
//	g_pDialogList->RemoveFromList(this);
//	::PostQuitMessage(0);
	return true;
}







void CFolderPropertiesDlg::DisplayPath(HWND hDlg)
{
	TCHAR szMessage[256] = {0};
	TCHAR szPath[MAX_PATH] = {0};
	CSettings::GetXmlFilePath(szPath, ARRAYSIZE(szPath));
	//*strrchr(szPath, '\\') = 0;
	::SetDlgItemText(hDlg, IDC_PATH, szPath);
}

void CFolderPropertiesDlg::DisplayVersion(HWND hDlg)
{
	TCHAR szFullPath[MAX_PATH] = {0};
	::GetModuleFileName(g_hInst, szFullPath, MAX_PATH);

	DWORD dwVerInfoSize;
	DWORD dwVerHnd=0;
	dwVerInfoSize = ::GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	if(dwVerInfoSize)
	{
		LPSTR   lpstrVffInfo;
		HANDLE  hMem;
		hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo = (LPSTR)::GlobalLock(hMem);
		::GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);

		UINT uVersionLen = 0;
		LPSTR lpVersion = NULL;
		TCHAR szGetName[256];
//		lstrcpy(szGetName, "\\StringFileInfo\\040904b0\\");	 
//		lstrcat(szGetName, TEXT("ProductVersion"));
		lstrcpy(szGetName, "\\StringFileInfo\\040904b0\\ProductVersion");	 
		BOOL bRetCode = ::VerQueryValue(
			(LPVOID)lpstrVffInfo,
			(LPSTR)szGetName,
			(LPVOID*)&lpVersion,
			(UINT *)&uVersionLen);

		if(bRetCode)
			::SetDlgItemText(hDlg, IDC_VERSION, lpVersion);

		::GlobalUnlock(hMem);
		::GlobalFree(hMem);
	}
}




bool CFolderPropertiesDlg::Equals(CFolderPropertiesDlg *pDlg)
{
	return (TRUE == CPidlManager::Equal(pDlg->m_pidl, this->m_pidl));
}

void CFolderPropertiesDlg::Show()
{
	UINT nRes = NULL == (LPITEMIDLIST)m_pidl ? IDD_ZENFOLDERS_INFO : IDD_FOLDER_PROPERTIES;

	m_hDlg = ::CreateDialogParam(
		g_hInst,
		MAKEINTRESOURCE(nRes), 
		NULL, //HWND hWndParent
		DlgProc,
		(LPARAM)this);

	::SetWindowLong(m_hDlg, DWL_USER, (LONG)this);

	::ShowWindow(m_hDlg, SW_SHOW) ;

	::SetFocus( GetDlgItem(m_hDlg, IDC_FOLDER_NAME) );
}

void CFolderPropertiesDlg::BringToFront()
{
//	::SetFocus( m_hDlg );
//	::ShowWindow(m_hDlg, SW_SHOW) ;
	::BringWindowToTop( m_hDlg );
}
/*
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

	//if (GetDlgCtrlID((HWND) wParam) != IDC_FOLDER_NAME) 
	//{ 
	//	SetFocus(GetDlgItem(hDlg, IDC_FOLDER_NAME)); 
	//	return FALSE; 
	//}
}
*/
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
	::SetDlgItemText(hDlg, IDC_FOLDER_QUERY, pData->searchData.szQuery);
}

void CFolderPropertiesDlg::InitMaxResults(HWND hDlg)
{
	LPPIDLDATA pData = m_pidl.GetData();
	::SetDlgItemInt(hDlg, IDC_MAXRESULTS, pData->searchData.maxResults, FALSE);
}

void CFolderPropertiesDlg::InitRanking(HWND hDlg)
{
	UINT radioButton = IDC_RANKING_RELEVANCE;

	LPPIDLDATA pData = m_pidl.GetData();

	if(pData->searchData.ranking == RECENCY_RANKING)
		radioButton = IDC_RANKING_RECENCY;

	::CheckRadioButton(
		hDlg,
		IDC_RANKING_RELEVANCE,
		IDC_RANKING_RECENCY,
		radioButton) ;
}

void CFolderPropertiesDlg::InitCategory(HWND hDlg)
{
	HWND hCombo = ::GetDlgItem(hDlg, IDC_CATEGORY);

	::SendMessage(hCombo, CB_LIMITTEXT , 20, 0);  

	LPCTSTR categories[] = 
	{
		"all", "email", "web", "im", "file", "contact",
		"calendar", "task", "note", "journal"
	};
	int n = sizeof(categories) / sizeof(LPCTSTR);
	for(int i=0; i<n; i++)
	{
		::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)categories[i]);
	}

	int selIndex = 0;
//	int selIndex = 3; // file

	LPPIDLDATA pData = m_pidl.GetData();

	int index = ::SendMessage(hCombo, CB_FINDSTRING, -1, (LPARAM)pData->searchData.szCategory);
	if(CB_ERR != index)
		selIndex = index;

	::SendMessage(hCombo, CB_SETCURSEL , selIndex, 0); 
}







bool CFolderPropertiesDlg::ApplyChanges(HWND hDlg)
{
	if(NULL == m_pParent)
		return false;

	if(NULL == (LPITEMIDLIST)m_pidl)
		return true;

	::SetCursor( ::LoadCursor(NULL, IDC_WAIT) );

	bool bSave = false;
	bool bResult = false;
	bool bName = false;

	PIDLDATA dataNew;
	LPPIDLDATA pDataOld = m_pidl.GetData();
	memcpy(&dataNew, pDataOld, sizeof(PIDLDATA));

	if( SetMaxResults(hDlg, &dataNew) )
		bSave = true;

	if( SetRanking(hDlg, &dataNew) )
		bSave = true;

	if( SetCategory(hDlg, &dataNew) )
		bSave = true;

	if( SetQueryString(hDlg, &dataNew) )
		bSave = true;

	if( SetFolderName(hDlg, &dataNew) )
	{
		bName = bSave = true;
	}

	if(bSave)
	{
		CPidl pidlNew( m_pidl.GetRelative() );
		LPPIDLDATA pDataTmp = pidlNew.GetData();
		memcpy(pDataTmp, &dataNew, sizeof(PIDLDATA));
		CPidl pidlFQNew = m_pParent->CreateFQPidl( pidlNew );
		if( g_pConfigXML->SaveFolder(m_pidl, pidlFQNew) )
		{
			TRACE_PIDL_PATH("CFolderPropertiesDlg::ApplyChanges pidlFQOld: %s\n", m_pidl);
			TRACE_PIDL_PATH("CFolderPropertiesDlg::ApplyChanges pidlFQNew: %s\n", pidlFQNew);

			if(bName)
			{
				::SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_IDLIST, m_pidl, pidlFQNew);
				SetCaption(m_hDlg, &dataNew);
			}
			else
				::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlFQNew, NULL);

			g_pViewList->Refresh();
			bResult = true;
		}
	}

	::SetCursor( LoadCursor(NULL, IDC_ARROW) );

	return bResult;
}

bool CFolderPropertiesDlg::SetFolderName(HWND hDlg, LPPIDLDATA pDataNew)
{
	UINT uLen;
	bool bSave = false;
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
			bSave = true;
		}
	}

	return bSave;
}

bool CFolderPropertiesDlg::SetQueryString(HWND hDlg, LPPIDLDATA pDataNew)
{
	UINT uLen;
	bool bSave = false;
	TCHAR szNewQuery[200] = {0};
	TCHAR attribName[] = _T("query");

	uLen = ::GetDlgItemText(
		hDlg,
		IDC_FOLDER_QUERY,
		szNewQuery,
		sizeof(szNewQuery)/sizeof(TCHAR));

	LPPIDLDATA pData = m_pidl.GetData();

	if(0 != lstrcmp(pData->searchData.szQuery, szNewQuery))
	{
		int len = sizeof(pData->searchData.szQuery)/sizeof(TCHAR);
		lstrcpyn(pDataNew->searchData.szQuery, szNewQuery, len);
		bSave = true;
	}

	return bSave;
}

bool CFolderPropertiesDlg::SetMaxResults(HWND hDlg, LPPIDLDATA pDataNew)
{
	UINT maxResults;
	BOOL bTranslated;
	bool bSave = false;
	TCHAR attribName[] = _T("maxResults");

	maxResults = ::GetDlgItemInt(hDlg, IDC_MAXRESULTS, &bTranslated, FALSE);

	if(!bTranslated)
		maxResults = 0;

	LPPIDLDATA pData = m_pidl.GetData();

	if(maxResults != pData->searchData.maxResults)
	{
		pDataNew->searchData.maxResults = maxResults;
		bSave = true;
	}

	return bSave;
}

bool CFolderPropertiesDlg::SetRanking(HWND hDlg, LPPIDLDATA pDataNew)
{
	bool bSave = false;
	UINT ranking = RELEVANCE_RANKING;

	if( !IsDlgButtonChecked(hDlg, IDC_RANKING_RELEVANCE) )
		ranking = RECENCY_RANKING;

	LPPIDLDATA pData = m_pidl.GetData();
	if(pData->searchData.ranking != ranking)
	{
		pDataNew->searchData.ranking = ranking;
		bSave = true;
	}

	return bSave;
}

bool CFolderPropertiesDlg::SetCategory(HWND hDlg, LPPIDLDATA pDataNew)
{
	bool bSave = false;
	TCHAR szCategory[21] = {0};

	HWND hCombo = GetDlgItem(hDlg, IDC_CATEGORY);

	//int index = SendMessage(hCombo, CB_GETCURSEL , 0, 0);
	//SendMessage(hCombo, CB_GETLBTEXT , index, (LPARAM)szCategory);
	::GetWindowText(hCombo, szCategory, 20);

	UINT uLen = lstrlen(szCategory);

	LPPIDLDATA pData = m_pidl.GetData();
	if(0 != lstrcmp(pData->searchData.szCategory, szCategory))
	{
		int size = sizeof(pData->searchData.szCategory);

		if(0 == lstrcmpi(szCategory, "all"))
		{
			memset(pDataNew->searchData.szCategory, 0, size);
		}
		else
		{
			int len = size / sizeof(TCHAR);
			lstrcpyn(pDataNew->searchData.szCategory, szCategory, len);
		}

		bSave = true;
	}

	return bSave;
}







void CFolderPropertiesDlg::SetCaption(HWND hDlg, LPPIDLDATA pidl)
{
	TCHAR szCaption[100] = {0};
	sprintf(szCaption, "%s Properties", pidl->szName);
	::SetWindowText(hDlg, szCaption);
}
