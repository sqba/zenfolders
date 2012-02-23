// cfgxml.cpp: implementation of the CConfigXML class.
//
//////////////////////////////////////////////////////////////////////

#include <tchar.h>
#include <crtdbg.h>
#include "cfgxml.h"
#include "util/string.h"
#include "util/settings.h"
#include "google/googleds.h"
#include "listview.h"


//#define TESTHR( hr )			{ if FAILED(hr) throw(hr); }
#define ARRAYSIZE(a)			(sizeof(a)/sizeof(a[0]))

#define MSXML_CLASS				"Msxml2.DOMDocument.3.0"

#define EMPTY_XML				"<root name='root'/>"

#define XPATH_FOLDER			"folder[@name=\"%s\"]"
#define XPATH_LINK				"link[@name=\"%s\" and @type=\"%s\"]"

#define ELEMENT_FOLDER			_T("folder")
#define ELEMENT_LINK			_T("link")

#define LINK_TYPE_FOLDER		_T("folder")
#define LINK_TYPE_FILE			_T("file")

#define ATTRIB_NAME				_T("name")
#define ATTRIB_QUERY			_T("query")
#define ATTRIB_CATEGORY			_T("category")
#define ATTRIB_RANKING			_T("ranking")
#define ATTRIB_MAXRESULTS		_T("maxResults")
#define ATTRIB_STYLE			_T("style")
#define ATTRIB_PATH				_T("path")
#define ATTRIB_TYPE				_T("type")

#define CATEGORY_FILE			_T("file")

#define STR_RECENCY				TEXT("recency")
#define STR_RELEVANCE			TEXT("relevance")

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

bool CConfigXML::Save()
{
	HRESULT hr = m_pXmlDoc->save(m_szFullPath);
	//return ( !FAILED(hr) );
	if( FAILED(hr) )
	{
		_RPTF1(_CRT_ERROR, "Failed to save %s\n", m_szFullPath);
		return false;
	}
	else
	{
		_RPTF1(_CRT_WARN, "Saved %s\n", m_szFullPath);
		return true;
	}
}

bool CConfigXML::SaveFolder(LPCITEMIDLIST pidlOld, LPCITEMIDLIST pidlNew)
{
	MSXML2::IXMLDOMNodePtr ptrNode = GetNode(pidlOld);
	LPCITEMIDLIST pidlRel = CPidlManager::GetLastItem(pidlNew);
	LPPIDLDATA pDataNew = CPidlManager::GetDataPointer(pidlRel);
	SetFolderInfo(ptrNode, pDataNew);

	if( !CPidlManager::Equal(pidlOld, pidlNew) )
		return Save();
	else
		return false;
}

bool CConfigXML::SaveFolder(LPCITEMIDLIST pidlNew)
{
	CPidl pidl(pidlNew);
	MSXML2::IXMLDOMNodePtr ptrNode = pidl.GetNode();
	LPPIDLDATA pData = pidl.GetData();
	SetFolderInfo(ptrNode, pData);
	return Save();
}

MSXML2::IXMLDOMNodePtr CConfigXML::CreateFolder(LPCTSTR pszName,
												MSXML2::IXMLDOMNodePtr parent)
{
	MSXML2::IXMLDOMElementPtr child; 
	child = m_pXmlDoc->createElement(ELEMENT_FOLDER);
	child->setAttribute(ATTRIB_NAME, pszName);
	child->setAttribute(ATTRIB_CATEGORY, CATEGORY_FILE);
	parent->appendChild(child);
	Save();
	return child;
}

bool CConfigXML::MoveFolder(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlNewParent)
{
	MSXML2::IXMLDOMNodePtr ptrNode = GetNode(pidl);
	if(NULL == ptrNode)
		return false;
	MSXML2::IXMLDOMNodePtr ptrParentNode = GetNode(pidlNewParent);
	if(NULL == ptrParentNode)
		return false;
	ptrParentNode->appendChild(ptrNode);
	return Save();
}

MSXML2::IXMLDOMNodePtr CConfigXML::GetSubfolder(MSXML2::IXMLDOMNodePtr node,
												LPCTSTR pszName)
{
	if(NULL == node)
		return NULL;
	TCHAR szXPath[MAX_QUERY] = {0};
	MSXML2::IXMLDOMNodePtr child;
	wsprintf(szXPath, XPATH_FOLDER, pszName);
	_bstr_t bsXPath(szXPath);
	try
	{
		return node->selectSingleNode(bsXPath);
	}
	catch(...)
	{
		return NULL;
	}
}

bool CConfigXML::DeleteFolder(LPCITEMIDLIST pidl)
{
	MSXML2::IXMLDOMNodePtr ptrNode = GetNode(pidl);
	if((NULL == ptrNode) || (NULL == ptrNode->parentNode))
		return false;
	ptrNode->parentNode->removeChild(ptrNode);
	return Save();
}

MSXML2::IXMLDOMNodePtr CConfigXML::GetNode(LPCITEMIDLIST pidl)
{
	if( CPidlManager::IsRoot(pidl) && !Load() )
		return NULL;

	TCHAR szXPath[MAX_PATH*2] = {0};

	if(0 == GetNodeXPath(pidl, szXPath, ARRAYSIZE(szXPath)))
		return NULL;

	try
	{
		MSXML2::IXMLDOMNodePtr ptrNode;
		ptrNode = m_pXmlDoc->selectSingleNode(szXPath);
		if(NULL == ptrNode)
		{
			_RPTF1(_CRT_WARN, _T("%s not found\n"), szXPath);
		}
		return ptrNode;
	}
	//catch(HRESULT hr)
	catch(...)
	{
		_RPTF1(_CRT_ERROR, "selectSingleNode(%s) failed\n", szXPath);
		return NULL;
	}
}

MSXML2::IXMLDOMNodePtr CConfigXML::CreateFolderLink(LPCTSTR lpszPath,
													MSXML2::IXMLDOMNodePtr parent)
{
	MSXML2::IXMLDOMElementPtr child;

	child = m_pXmlDoc->createElement(ELEMENT_LINK);
	child->setAttribute(ATTRIB_TYPE, LINK_TYPE_FOLDER);
	child->setAttribute(ATTRIB_NAME, CString::GetFilename(lpszPath));
	child->setAttribute(ATTRIB_PATH, lpszPath);
	parent->appendChild(child);
	Save();

	return child;
}

MSXML2::IXMLDOMNodePtr CConfigXML::CreateFileLink(LPCTSTR lpszPath,
												  MSXML2::IXMLDOMNodePtr parent)
{
	MSXML2::IXMLDOMElementPtr child;

	child = m_pXmlDoc->createElement(ELEMENT_LINK);
	child->setAttribute(ATTRIB_TYPE, LINK_TYPE_FILE);
	child->setAttribute(ATTRIB_PATH, lpszPath);
	parent->appendChild(child);
	Save();

	return child;
}

MSXML2::IXMLDOMNodeListPtr CConfigXML::GetFolders(MSXML2::IXMLDOMNodePtr node)
{
	return node->selectNodes(_bstr_t(ELEMENT_FOLDER));
}

MSXML2::IXMLDOMNodeListPtr CConfigXML::GetFolderLinks(MSXML2::IXMLDOMNodePtr node)
{
	return node->selectNodes(_bstr_t("link[@type=\"folder\"]"));
}

MSXML2::IXMLDOMNodeListPtr CConfigXML::GetFileLinks(MSXML2::IXMLDOMNodePtr node)
{
	return node->selectNodes(_bstr_t("link[@type=\"file\"]"));
}









//////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////

bool CConfigXML::Load()
{
	HRESULT hr = m_pXmlDoc.CreateInstance( MSXML_CLASS );
	if( FAILED(hr) )
	{
		_RPTF1(_CRT_ERROR, "Failed to create %s.\n", MSXML_CLASS);
		return false;
	}
	
	_variant_t varXml(m_szFullPath);
	VARIANT_BOOL varOut = m_pXmlDoc->load(varXml);

	if(VARIANT_TRUE == varOut)
	{
		_RPTF1(_CRT_WARN, "Loaded %s\n", m_szFullPath);
		return true;
	}
	else
	{
		_RPTF1(_CRT_WARN, "Failed to load '%s', creating new.\n", m_szFullPath);
		_bstr_t bsXML(TEXT(EMPTY_XML));
		varOut = m_pXmlDoc->loadXML(bsXML);
		if(VARIANT_TRUE == varOut)
		{
			_RPTF0(_CRT_WARN, "Created configuration\n");
			return Save();
		}
		else
		{
			_RPTF0(_CRT_ERROR, "Failed to create configuration\n");
			return false;
		}
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
		switch(pData->type)
		{
		case PT_FILE:
			break;

		case PT_FOLDER:
			memset(szTemp, 0, ARRAYSIZE(szTemp));
			lstrcpy(szTemp, pData->szName);
			EscapeXML(szTemp, lstrlen(szTemp));
			wsprintf(szBuffer, XPATH_FOLDER, szTemp);
			break;

		case PT_FOLDER_LINK:
			memset(szTemp, 0, ARRAYSIZE(szTemp));
			lstrcpy(szTemp, pData->szName);
			EscapeXML(szTemp, lstrlen(szTemp));
			wsprintf(szBuffer, XPATH_LINK, szTemp, LINK_TYPE_FOLDER);
			break;

		default: // Root
			wsprintf(szBuffer, "*");
		}

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









//////////////////////////////////////////////////////////////////////
// Static functions
//////////////////////////////////////////////////////////////////////

bool CConfigXML::GetFolderInfo(LPPIDLDATA pData, MSXML2::IXMLDOMNodePtr node)
{
	UINT			size;
	UINT			uTmpSize = 0;
	DWORD			dwLen;
	TCHAR			szRanking[21] = {0};
	UINT			ranking = RELEVANCE_RANKING;
	LPSEARCHDATA	pSearchData = &pData->searchData;

	size = ARRAYSIZE(pData->szName);
	dwLen = GetNodeAttribute(node, ATTRIB_NAME, pData->szName, size);
	if(dwLen == 0)
		return false;

	size = ARRAYSIZE(pSearchData->szCategory);
	memset(pSearchData->szCategory, 0, size);
	dwLen = GetNodeAttribute(node, ATTRIB_CATEGORY, pSearchData->szCategory, size);

	pSearchData->maxResults = GetNodeAttribute(node, ATTRIB_MAXRESULTS, DEFAULT_MAX_RESULTS);

	size = ARRAYSIZE(szRanking);
	dwLen = GetNodeAttribute(node, ATTRIB_RANKING, szRanking, size);
	if(dwLen > 0)
	{
		if(0 == lstrcmpi(szRanking, STR_RECENCY))
			pSearchData->ranking = RECENCY_RANKING;
	}

	size = ARRAYSIZE(pSearchData->szQuery);
	memset(pSearchData->szQuery, 0, sizeof(pSearchData->szQuery));
	dwLen = GetNodeAttribute(node, ATTRIB_QUERY, pSearchData->szQuery, size);

	pSearchData->viewStyle = (LONG)GetNodeAttribute(node, ATTRIB_STYLE, (INT)LVS_REPORT2);

	return true;
}

bool CConfigXML::SetFolderInfo(MSXML2::IXMLDOMNodePtr node, LPPIDLDATA pData)
{
	// Folder name
	SetNodeAttribute(
		node,
		ATTRIB_NAME,
		pData->szName,
		lstrlen(pData->szName));

	// Folder query string
	SetNodeAttribute(
		node,
		ATTRIB_QUERY,
		pData->searchData.szQuery,
		lstrlen(pData->searchData.szQuery));

	// Category
	SetNodeAttribute(
		node,
		ATTRIB_CATEGORY,
		pData->searchData.szCategory,
		lstrlen(pData->searchData.szCategory));

	// Ranking by
	UINT nRanking = pData->searchData.ranking;
	LPCTSTR lpszRanking = (RELEVANCE_RANKING == nRanking ? STR_RELEVANCE : STR_RECENCY);
	SetNodeAttribute(
		node,
		ATTRIB_RANKING,
		lpszRanking,
		lstrlen(lpszRanking));
//	_RPTF1(_CRT_WARN, "pData->searchData.ranking: %s\n", szRanking);

	// Maximum number of results
	SetNodeAttribute(
		node,
		ATTRIB_MAXRESULTS,
		pData->searchData.maxResults);
//	_RPTF1(_CRT_WARN, "pData->searchData.maxResults: %d\n", pData->searchData.maxResults);

	SetNodeAttribute(
		node,
		ATTRIB_STYLE,
		(INT)pData->searchData.viewStyle);

	return true;
}

bool CConfigXML::GetFolderLinkInfo(LPPIDLDATA pData, MSXML2::IXMLDOMNodePtr node)
{
	UINT			size;
	DWORD			dwLen;
	LPFILEDATA		pFileData = &pData->fileData;

	size = ARRAYSIZE(pData->szName);
	memset(pData->szName, 0, size);
	size /= sizeof(TCHAR);
	dwLen = GetNodeAttribute(node, ATTRIB_NAME, pData->szName, size);
	if(dwLen == 0)
		return false;

	size = ARRAYSIZE(pFileData->szPath);
	memset(pFileData->szPath, 0, size);
	size /= sizeof(TCHAR);
	dwLen = GetNodeAttribute(node, ATTRIB_PATH, pFileData->szPath, size);

	return (dwLen > 0);
}

bool CConfigXML::SetNodeAttribute(MSXML2::IXMLDOMNodePtr node,
								  LPCTSTR attribName,
								  LPCTSTR pszValue,
								  USHORT uSize)
{
	if(node == NULL)
		return false;

	try
	{
		MSXML2::IXMLDOMElementPtr ptrNode(node);
		ptrNode->setAttribute(attribName, pszValue);
		return true;
	}
	catch(...)
	{
		_RPTF0(_CRT_ERROR, _T("SetNodeAttribute failed\n"));
		return false;
	}
}

bool CConfigXML::SetNodeAttribute(MSXML2::IXMLDOMNodePtr node,
								  LPCTSTR attribName,
								  UINT value)
{
	if(node == NULL)
		return false;

	try
	{
		MSXML2::IXMLDOMElementPtr ptrNode(node);
		TCHAR szBuffer[20] = {0};
		itoa(value, szBuffer, 10);
		ptrNode->setAttribute(attribName, szBuffer);
		return true;
	}
	catch(...)
	{
		_RPTF0(_CRT_ERROR, _T("SetNodeAttribute failed\n"));
		return false;
	}
}

UINT CConfigXML::GetNodeName(MSXML2::IXMLDOMNodePtr node,
							 LPTSTR pszTemp,
							 USHORT uSize)
{
	if(node == NULL)
		return 0;

	BSTR nodeName;
	WCHAR *wcNodeName;
	HRESULT hr = node->get_nodeName(&nodeName);
	if( FAILED(hr) )
	{
		_RPTF0(_CRT_WARN, "GetNodeName failed.\n");
		return 0;
	}
	wcNodeName = (wchar_t*)nodeName;
	CString::WideCharToLocal(pszTemp, wcNodeName, uSize);
	return wcslen(wcNodeName);
}

UINT CConfigXML::GetNodeValue(MSXML2::IXMLDOMNodePtr node,
							  LPTSTR pszTemp,
							  USHORT uSize)
{
	if(node == NULL)
		return 0;
	
	VARIANT value;
	HRESULT hr = node->get_nodeValue(&value);
	if( FAILED(hr) )
	{
		_RPTF0(_CRT_WARN, "GetNodeValue failed.\n");
		return 0;
	}
	CString::WideCharToLocal(pszTemp, value.bstrVal, uSize);
	return wcslen(value.bstrVal);
}

UINT CConfigXML::GetNodeAttribute(MSXML2::IXMLDOMNodePtr node,
								  LPCTSTR attribName,
								  LPTSTR pszTemp,
								  USHORT uSize)
{
	if(node == NULL)
		return 0;

	try
	{
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

	try
	{
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

bool CConfigXML::HasSubFolders(LPCITEMIDLIST pidl)
{
	try
	{
		MSXML2::IXMLDOMNodePtr ptrNode = GetNode(pidl);
		LPPIDLDATA pData = CPidlManager::GetDataPointer(pidl);
		if(NULL == ptrNode)
			return false;
		MSXML2::IXMLDOMNodeListPtr subfolders;
		subfolders = ptrNode->selectNodes(_bstr_t(ELEMENT_FOLDER));
		return (subfolders->Getlength() > 0);
	}
	catch(...)
	{
		_RPTF0(_CRT_WARN, "CPidlManager::HasChildNodes exception\n");
		return false;
	}
}
