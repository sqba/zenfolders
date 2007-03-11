// cfgxml.cpp: implementation of the CConfigXML class.
//
//////////////////////////////////////////////////////////////////////

#include <tchar.h>
#include <crtdbg.h>
#include "cfgxml.h"
#include "util/string.h"
#include "util/settings.h"
#include "google/googleds.h"

#define TESTHR( hr )	{ if FAILED(hr) throw(hr); }
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfigXML::CConfigXML()
{
	CSettings::GetXmlFilePath(m_szFullPath, ARRAYSIZE(m_szFullPath));
	Load();
}

CConfigXML::~CConfigXML()
{

}

BOOL CConfigXML::Load()
{
	try 
	{
		TESTHR( m_pXmlDoc.CreateInstance("Msxml2.DOMDocument.3.0") );
		
		_variant_t varXml(m_szFullPath);
		VARIANT_BOOL varOut = m_pXmlDoc->load(varXml);
	
		if(VARIANT_TRUE == varOut)
		{
			_RPTF1(_CRT_WARN, "Loaded %s\n", m_szFullPath);
			return TRUE;
		}
		else
		{
			_RPTF1(_CRT_WARN, "Failed to load %s\n", m_szFullPath);
			_bstr_t bsXML(TEXT("<root name='root'/>"));
			varOut = m_pXmlDoc->loadXML(bsXML);
			if(VARIANT_TRUE == varOut)
			{
				_RPTF0(_CRT_WARN, "Created configuration\n");
				return TRUE;
			}
			else
			{
				_RPTF0(_CRT_ERROR, "Failed to create configuration\n");
				return FALSE;
			}
		}
	}
	catch(...)
	{
		_RPTF1(_CRT_ERROR, "Exception trying to load %s\n", m_szFullPath);
		return FALSE;
	}
}

BOOL CConfigXML::Save()
{
	HRESULT hr = m_pXmlDoc->save(m_szFullPath);
	if( FAILED(hr) )
	{
		_RPTF1(_CRT_ERROR, "Failed to save %s\n", m_szFullPath);
		return FALSE;
	}
	else
	{
		_RPTF1(_CRT_WARN, "CConfigXML::SaveXML(%s)\n", m_szFullPath);
		return TRUE;
	}
	//return ( !FAILED(hr) );
}

BOOL CConfigXML::SaveFolder(LPCITEMIDLIST pidlOld, LPCITEMIDLIST pidlNew)
{
	MSXML2::IXMLDOMNodePtr ptrNode = GetNode(pidlOld);
	LPCITEMIDLIST pidlRel = CPidlManager::GetLastItem(pidlNew);
	LPPIDLDATA pDataNew = CPidlManager::GetDataPointer(pidlRel);
	SetFolderInfo(ptrNode, pDataNew);

//	MSXML2::IXMLDOMNodePtr ptrNodeTmp = GetNode(pidlOld);
//	TCHAR szDebug[100] = {0};
//	GetNodeAttribute(ptrNodeTmp, _T("query"), szDebug, sizeof(szDebug)/sizeof(TCHAR));

	if( !CPidlManager::Equal(pidlOld, pidlNew) )
		return Save();
	else
		return FALSE;
}

BOOL CConfigXML::SaveFolder(LPCITEMIDLIST pidlNew)
{
	CPidl pidl(pidlNew);
	MSXML2::IXMLDOMNodePtr ptrNode = pidl.GetNode();
	LPPIDLDATA pData = pidl.GetData();
	SetFolderInfo(ptrNode, pData);
	return Save();
}

MSXML2::IXMLDOMNodePtr CConfigXML::GetRoot()
{
	return m_pXmlDoc->documentElement;
}

MSXML2::IXMLDOMNodePtr CConfigXML::CreateFolder(LPCTSTR pszName,
												MSXML2::IXMLDOMNodePtr parent)
{
	MSXML2::IXMLDOMElementPtr child; 
	child = m_pXmlDoc->createElement(_T("folder"));
	child->setAttribute(_T("name"), pszName);
	parent->appendChild(child);
	Save();
	return child;
}

BOOL CConfigXML::GetFolderInfo(LPPIDLDATA pData, MSXML2::IXMLDOMNodePtr node)
{
	UINT size;
	UINT uTmpSize = 0;
	DWORD dwLen;
	TCHAR szRanking[21] = {0};
	UINT ranking = RELEVANCE_RANKING;
	LPFOLDERDATA pFolderData = &pData->folderData;

	size = ARRAYSIZE(pData->szName);
	dwLen = GetNodeAttribute(node, _T("name"), pData->szName, size);
	if(dwLen == 0)
		dwLen = GetNodeName(node, pData->szName, size);

	size = ARRAYSIZE(pFolderData->szCategory);
	memset(pFolderData->szCategory, 0, size);
	dwLen = GetNodeAttribute(node, _T("category"), pFolderData->szCategory, size);

	pFolderData->maxResults = GetNodeAttribute(node, _T("maxResults"), DEFAULT_MAX_RESULTS);

	size = ARRAYSIZE(szRanking);
	dwLen = GetNodeAttribute(node, _T("ranking"), szRanking, size);
	if(dwLen > 0)
	{
		if(0 == lstrcmpi(szRanking, TEXT("recency")))
			pFolderData->ranking = RECENCY_RANKING;
	}

	size = ARRAYSIZE(pFolderData->szQuery);
	memset(pFolderData->szQuery, 0, size);
	dwLen = GetNodeAttribute(node, _T("query"), pFolderData->szQuery, size);

	pFolderData->viewStyle = (LONG)GetNodeAttribute(node, _T("style"), (INT)LVS_REPORT);

	return TRUE;
}

BOOL CConfigXML::SetFolderInfo(MSXML2::IXMLDOMNodePtr node, LPPIDLDATA pData)
{
	// Folder name
	SetNodeAttribute(
		node,
		_T("name"),
		pData->szName,
		lstrlen(pData->szName));
//	_RPTF1(_CRT_WARN, "pData->szName: %s\n", pData->szName);

//	TCHAR szDebug[100] = {0};
//	GetNodeAttribute(pNode, _T("name"), szDebug, sizeof(szDebug)/sizeof(TCHAR));

	// Folder query string
	SetNodeAttribute(
		node,
		 _T("query"),
		pData->folderData.szQuery,
		lstrlen(pData->folderData.szQuery));
//	_RPTF1(_CRT_WARN, "pData->folderData.szQuery: %s\n", pData->folderData.szQuery);

//	GetNodeAttribute(pNode, _T("query"), szDebug, sizeof(szDebug)/sizeof(TCHAR));

	// Category
	SetNodeAttribute(
		node,
		 _T("category"),
		pData->folderData.szCategory,
		lstrlen(pData->folderData.szCategory));
//	_RPTF1(_CRT_WARN, "pData->folderData.szCategory: %s\n", pData->folderData.szCategory);

//	GetNodeAttribute(pNode, _T("category"), szDebug, sizeof(szDebug)/sizeof(TCHAR));

	// Ranking by
	TCHAR szRanking[21] = {0};
	if(RELEVANCE_RANKING == pData->folderData.ranking)
		strcpy(szRanking, TEXT("relevance"));
	else
		strcpy(szRanking, TEXT("recency"));
	SetNodeAttribute(
		node,
		_T("ranking"),
		szRanking,
		lstrlen(szRanking));
//	_RPTF1(_CRT_WARN, "pData->folderData.ranking: %s\n", szRanking);

	// Maximum number of results
	SetNodeAttribute(
		node,
		_T("maxResults"),
		pData->folderData.maxResults);
//	_RPTF1(_CRT_WARN, "pData->folderData.maxResults: %d\n", pData->folderData.maxResults);

	SetNodeAttribute(
		node,
		_T("style"),
		(INT)pData->folderData.viewStyle);

	return TRUE;
}

BOOL CConfigXML::RemoveFolder(LPCITEMIDLIST pidl)
{
	MSXML2::IXMLDOMNodePtr ptrNode = GetNode(pidl);
	ptrNode->parentNode->removeChild(ptrNode);
	return Save();
}

BOOL CConfigXML::MoveFolder(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlNewParent)
{
	MSXML2::IXMLDOMNodePtr ptrNode = GetNode(pidl);
	MSXML2::IXMLDOMNodePtr ptrParentNode = GetNode(pidlNewParent);
	ptrParentNode->appendChild(ptrNode);
	return Save();
}

MSXML2::IXMLDOMNodePtr CConfigXML::GetSubfolder(MSXML2::IXMLDOMNodePtr node,
												LPCTSTR pszName)
{
	TCHAR szQuery[MAX_QUERY] = {0};
	MSXML2::IXMLDOMNodePtr child;
	wsprintf(szQuery, "folder[@name='%s']", pszName);
	_bstr_t bsQuery(szQuery);
	try
	{
		child = node->selectSingleNode(bsQuery);
		return child;
	}
	catch(...)
	{
		return NULL;
	}
}


UINT CConfigXML::GetNodeName(MSXML2::IXMLDOMNodePtr node,
							 LPTSTR pszTemp,
							 USHORT uSize)
{
	if(node == NULL)
		return 0;
	try {
		BSTR nodeName;
		WCHAR *wcNodeName;
		TESTHR(node->get_nodeName(&nodeName));
		wcNodeName = (wchar_t*)nodeName;
		CString::WideCharToLocal(pszTemp, wcNodeName, uSize);
		return wcslen(wcNodeName);
	}
	catch(...)
	{
		_RPTF0(_CRT_WARN, "CConfigXML::GetNodeName exception\n");
		lstrcpy(pszTemp, TEXT("GetNodeName exception"));
		return strlen(pszTemp);
	}
}

UINT CConfigXML::GetNodeValue(MSXML2::IXMLDOMNodePtr node,
							  LPTSTR pszTemp,
							  USHORT uSize)
{
	if(node == NULL)
		return 0;
	try {
		VARIANT value;
		TESTHR(node->get_nodeValue(&value));
		CString::WideCharToLocal(pszTemp, value.bstrVal, uSize);
		return wcslen(value.bstrVal);
	}
	catch(...)
	{
		_RPTF0(_CRT_WARN, "CConfigXML::GetNodeValue exception\n");
		lstrcpy(pszTemp, TEXT("GetNodeValue exception"));
		return strlen(pszTemp);
	}
}

UINT CConfigXML::GetNodeAttribute(MSXML2::IXMLDOMNodePtr node,
								  LPCTSTR attribName,
								  LPTSTR pszTemp,
								  USHORT uSize)
{
	if(node == NULL)
		return 0;
	try {
		MSXML2::IXMLDOMNodePtr ptrAttrib;
		ptrAttrib = node->attributes->getNamedItem(attribName);
		if(ptrAttrib == NULL)
			return 0;
		return GetNodeValue(ptrAttrib, pszTemp, uSize);
	}
	catch(...)
	{
		_RPTF0(_CRT_WARN, "CConfigXML::GetNodeAttribute exception\n");
		lstrcpy(pszTemp, TEXT("GetNodeAttribute exception"));
		return strlen(pszTemp);
	}
}

UINT CConfigXML::GetNodeAttribute(MSXML2::IXMLDOMNodePtr node,
								  LPCTSTR attribName,
								  UINT defaultValue)
{
	if(node == NULL)
		return defaultValue;
	try {
		MSXML2::IXMLDOMNodePtr ptrAttrib;
		ptrAttrib = node->attributes->getNamedItem(attribName);
		if(ptrAttrib == NULL)
			return defaultValue;
		TCHAR szBuffer[20] = {0};
		if(0 >= GetNodeValue(ptrAttrib, szBuffer, ARRAYSIZE(szBuffer)))
			return defaultValue;
		return atoi(szBuffer);
	}
	catch(...)
	{
		_RPTF0(_CRT_WARN, _T("GetNodeAttribute exception\n"));
		return defaultValue;
	}
}

BOOL CConfigXML::SetNodeAttribute(MSXML2::IXMLDOMNodePtr node,
								  LPCTSTR attribName,
								  LPCTSTR pszValue,
								  USHORT uSize)
{
	if(node == NULL)
		return FALSE;

	try
	{
		MSXML2::IXMLDOMElementPtr ptrNode(node);
		ptrNode->setAttribute(attribName, pszValue);
		return TRUE;
	}
	catch(...)
	{
		_RPTF0(_CRT_ERROR, _T("SetNodeAttribute failed\n"));
		return FALSE;
	}
}

BOOL CConfigXML::SetNodeAttribute(MSXML2::IXMLDOMNodePtr node,
								  LPCTSTR attribName,
								  UINT value)
{
	if(node == NULL)
		return FALSE;

	try
	{
		MSXML2::IXMLDOMElementPtr ptrNode(node);
		TCHAR szBuffer[20] = {0};
		itoa(value, szBuffer, 10);
		ptrNode->setAttribute(attribName, szBuffer);
		return TRUE;
	}
	catch(...)
	{
		_RPTF0(_CRT_ERROR, _T("SetNodeAttribute failed\n"));
		return FALSE;
	}
}
/*
MSXML2::IXMLDOMNodePtr CConfigXML::CreateChild(MSXML2::IXMLDOMNode *node,
											   LPCTSTR pszName)
{
	VARIANT v;
	V_VT(&v) = VT_I4;
	V_I4(&v) = NODE_ELEMENT;
	MSXML2::IXMLDOMElementPtr ptrChild;
	ptrChild = m_pXmlDoc->createNode( v, pszName, "" );
	node->appendChild( ptrChild );
	return ptrChild;
}

void CConfigXML::CreateAttribute(MSXML2::IXMLDOMNode *node,
								 LPCTSTR pszName,
								 LPCTSTR pszValue)
{
	MSXML2::IXMLDOMAttributePtr ptrAttrib;
	ptrAttrib = m_pXmlDoc->createAttribute( pszName );
	ptrAttrib->nodeTypedValue = pszValue;
	node->attributes->setNamedItem( ptrAttrib );
}
*/
MSXML2::IXMLDOMNodePtr CConfigXML::GetNode(LPCITEMIDLIST pidl)
{
	TCHAR szXPath[MAX_PATH*2] = {0};

	if(0 == GetNodeXPath(pidl, szXPath, ARRAYSIZE(szXPath)))
		return NULL;

	try
	{
		MSXML2::IXMLDOMNodePtr ptrNode;
		ptrNode = m_pXmlDoc->selectSingleNode(szXPath);
		return ptrNode;
	}
	//catch(HRESULT hr)
	catch(...)
	{
		_RPTF1(_CRT_ERROR, "selectSingleNode(%s) failed\n", szXPath);
		return NULL;
	}
}

USHORT CConfigXML::EscapeXML(LPTSTR pszXML, USHORT uSize)
{
	TCHAR szBuffer[MAX_PATH] = { 0 };
	memset(szBuffer, 0, uSize * sizeof(TCHAR));

	UINT size = 0;

	for(int i = 0; pszXML[i]; i++)
	{
		if(size >= uSize)
			break;
		switch(pszXML[i])
		{
			  case '<':
				  lstrcat(szBuffer, "&lt;");
				  size += 4;
				  break;
			  case '>':
				  lstrcat(szBuffer, "&gt;");
				  size += 4;
				  break;
			  case '&':
				  lstrcat(szBuffer, "&amp;");
				  size += 5;
				  break;
			  case '"':
				  lstrcat(szBuffer, "&quot;");
				  size += 5;
				  break;
			  default:
				  szBuffer[size] = pszXML[i];
				  size++;
		}
	}

	lstrcpy(pszXML, szBuffer);

	return size;
}

USHORT CConfigXML::GetNodeXPath(LPCITEMIDLIST pidl,
								LPTSTR pszXPath,
								USHORT uSize)
{
	USHORT			uResultSize = 0;
	LPITEMIDLIST	pidlTemp;
	TCHAR			szBuffer[MAX_PATH] = {0};
	TCHAR			szTemp[MAX_PATH] = {0};

	pidlTemp = (LPITEMIDLIST)pidl;
	
	while(pidlTemp->mkid.cb)
	{
		LPPIDLDATA pData = CPidlManager::GetDataPointer(pidlTemp);
		if(PT_FOLDER == pData->type)
		{
			memset(szTemp, 0, sizeof(szTemp) * sizeof(TCHAR));
			lstrcpy(szTemp, pData->szName);
			EscapeXML(szTemp, lstrlen(szTemp));
			wsprintf(szBuffer, "folder[@name=\"%s\"]", szTemp);
		}
		else if(PT_FILE == pData->type)
			break;
		else // Root
			wsprintf(szBuffer, "*");

		USHORT len = lstrlen(szBuffer) + 1;
		if(uResultSize + len > uSize)
			break;

		lstrcat(pszXPath, "/");
		lstrcat(pszXPath, szBuffer);

		uResultSize += len;
		
		pidlTemp = CPidlManager::GetNextItem(pidlTemp);
	}

	return uResultSize;
}
