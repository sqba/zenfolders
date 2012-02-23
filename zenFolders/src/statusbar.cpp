// statusbar.cpp: implementation of the CStatusBar class.
//
//////////////////////////////////////////////////////////////////////

#include "statusbar.h"


void CStatusBar::Init(LPSHELLBROWSER pShellBrowser)
{
	LRESULT lResult;
	int nPartArray[] = {200, 400, -1};

	//set the number of parts
	pShellBrowser->SendControlMsg(
		FCW_STATUS,
		SB_SETPARTS,
		sizeof(nPartArray) / sizeof(int),
		(LPARAM)nPartArray,
		&lResult);
}

void CStatusBar::Fill(LPSHELLBROWSER pShellBrowser,
					  CShellFolder *pSFParent)
{
	LPITEMIDLIST pidlRel = pSFParent->GetPidlRel();

	if(NULL == pidlRel)
		return;

	if( CPidlManager::IsFile(pidlRel) )
		return;

	LPPIDLDATA pData = CPidlManager::GetDataPointer(pidlRel);
	if(NULL == pData)
		return;

	TCHAR szInfo[MAX_PATH] = {0};
	int files = pSFParent->GetFileCount();
	int folders = pSFParent->GetSubFolderCount();

	if((files > 0) && (folders > 0))
		wsprintf(szInfo, TEXT("%d file(s) %d folder(s)"), files, folders);
	else if(files > 0)
		wsprintf(szInfo, TEXT("%d file(s)"), files);
	else if(folders > 0)
		wsprintf(szInfo, TEXT("%d folder(s)"), folders);
	else
		wsprintf(szInfo, TEXT("Empty"));
	
	SetText(0, pData->szName, pShellBrowser);
	SetText(1, szInfo, pShellBrowser);
	SetText(2, pData->searchData.szQuery, pShellBrowser);
}

void CStatusBar::SetText(int index,
						 LPCTSTR lpszText,
						 LPSHELLBROWSER pShellBrowser)
{
	LRESULT lResult;

	pShellBrowser->SendControlMsg(
		FCW_STATUS,
		SB_SETTEXT,
		index,
		(LPARAM)lpszText,
		&lResult);
}
