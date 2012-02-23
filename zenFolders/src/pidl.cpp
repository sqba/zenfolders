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
	return m_pidl ? (TRUE == CPidlManager::Equal(m_pidl, pidl)) : false;
}

bool CPidl::operator != (LPCITEMIDLIST pidl)
{
	return m_pidl ? (FALSE == CPidlManager::Equal(m_pidl, pidl)) : false;
}

LPITEMIDLIST CPidl::GetRelative()
{
	return m_pidl ? CPidlManager::GetLastItem(m_pidl) : NULL;
}

MSXML2::IXMLDOMNodePtr CPidl::GetNode()
{
	return g_pConfigXML->GetNode(m_pidl);
}

LPPIDLDATA CPidl::GetData()
{
	return m_pidl ? CPidlManager::GetDataPointer( GetRelative() ) : NULL;
}

bool CPidl::IsFile()
{
	return CPidlManager::IsFile( m_pidl );
}

bool CPidl::IsFolder()
{
	return CPidlManager::IsFolder( m_pidl );
}

bool CPidl::IsFolderLink()
{
	return CPidlManager::IsFolderLink( m_pidl );
}

bool CPidl::IsRoot()
{
	return CPidlManager::IsRoot( m_pidl );
}

bool CPidl::IsSubFolder()
{
	return CPidlManager::IsSubFolder( m_pidl );
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
	if( !IsFile() && !IsSubFolder() )
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

CPidl::operator LPITEMIDLIST()
{
	return m_pidl;
}

CPidl::operator LPPIDLDATA()
{
	return GetData();
}

CPidl::operator LPVOID()
{
	return m_pidl;
}
