#include "pidlmgr.h"


extern LPPIDLMGR	g_pPidlMgr;
extern LPCONFIGXML	g_pConfigXML;


CPidl::CPidl()
{
	m_pidl = NULL;
}

CPidl::CPidl(LPCITEMIDLIST pidl)
{
	m_pidl = g_pPidlMgr->Copy(pidl);
}

CPidl::~CPidl()
{
	if(m_pidl)
		g_pPidlMgr->Delete(m_pidl);
}

void CPidl::operator = (LPCITEMIDLIST pidl)
{
	if(m_pidl)
		g_pPidlMgr->Delete(m_pidl);

	m_pidl = g_pPidlMgr->Copy(pidl);
}

LPITEMIDLIST CPidl::operator + (LPCITEMIDLIST pidl)
{
	return g_pPidlMgr->Concatenate(m_pidl, pidl);
}

LPITEMIDLIST CPidl::operator + (CPidl pidl)
{
	return g_pPidlMgr->Concatenate(m_pidl, pidl.m_pidl);
}

bool CPidl::operator == (LPCITEMIDLIST pidl)
{
	if(m_pidl)
		return (TRUE == CPidlManager::Equal(m_pidl, pidl));
	else
		return false;
}

bool CPidl::operator != (LPCITEMIDLIST pidl)
{
	if(m_pidl)
		return (FALSE == CPidlManager::Equal(m_pidl, pidl));
	else
		return false;
}

LPITEMIDLIST CPidl::GetRelative()
{
	if(m_pidl)
		return CPidlManager::GetLastItem(m_pidl);
	else
		return NULL;
}

LPITEMIDLIST CPidl::GetFull()
{
	return m_pidl;
}

MSXML2::IXMLDOMNodePtr CPidl::GetNode()
{
	return g_pConfigXML->GetNode(m_pidl);
}

LPPIDLDATA CPidl::GetData()
{
	if(m_pidl)
		return CPidlManager::GetDataPointer( GetRelative() );
	else
		return NULL;
}

BOOL CPidl::IsFile()
{
	if(0 == m_pidl->mkid.cb)
		return FALSE;
//	return CPidlManager::IsFile( GetRelative() );
	LPPIDLDATA pData = GetData();
	if(NULL == pData)
		return FALSE;
	return (PT_FILE == pData->type);
}

BOOL CPidl::IsFolder()
{
	if(0 == m_pidl->mkid.cb)
		return FALSE;
	LPPIDLDATA pData = GetData();
	if(NULL == pData)
		return FALSE;
	return (PT_FOLDER == pData->type);
}

BOOL CPidl::IsRoot()
{
//	return (0 == m_pidl->mkid.cb);
	LPPIDLDATA pData = GetData();
	if(NULL == pData)
		return FALSE;
	return ((PT_FOLDER != pData->type) && (PT_FILE != pData->type));
}

DWORD CPidl::GetName(LPTSTR lpszText, USHORT uSize)
{
	return g_pPidlMgr->GetItemName(GetRelative(), lpszText, uSize);
}

DWORD CPidl::GetPath(LPTSTR lpszOut, DWORD dwOutSize)
{
	return g_pPidlMgr->GetPidlPath(GetRelative(), lpszOut, dwOutSize);
}

DWORD CPidl::GetFSPath(LPTSTR lpszOut, DWORD dwOutSize)
{
	if( !IsFile() )
		return 0;
	LPPIDLDATA pData = GetData();
	lstrcpyn(lpszOut, pData->fileData.szPath, dwOutSize);
	return lstrlen(lpszOut);
}

LPITEMIDLIST CPidl::operator += (LPCITEMIDLIST pidl)
{
	g_pPidlMgr->Concatenate(m_pidl, pidl);
	return m_pidl;
}
