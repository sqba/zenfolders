
#include <crtdbg.h>
#include <tchar.h>
//#include <strsafe.h>

#include "enumidl.h"
#include "shlfldr.h"
#include "cfgxml.h"

#include "google/GoogleDS.h"
#include "util/string.h"
#include "zenfolders.h"


//#define IS_DIRECTORY(ffd)	((FILE_ATTRIBUTE_DIRECTORY & ffd.dwFileAttributes) > 0)

extern HINSTANCE	g_hInst;
extern LPCONFIGXML	g_pConfigXML;
extern LPPIDLMGR	g_pPidlMgr;


CEnumIDList::CEnumIDList(LPCITEMIDLIST pidl,
						 DWORD dwFlags,
						 HRESULT *pResult)
{
	if(pResult)
		*pResult = S_OK;
	
	m_pFirst = m_pLast = m_pCurrent = NULL;

	m_iFileCount = m_iFolderCount = 0;

	//get the shell's IMalloc pointer
	//we'll keep this until we get destroyed
	if( FAILED(::SHGetMalloc(&m_pMalloc)) )
	{
		if(pResult)
			*pResult = E_OUTOFMEMORY;
		
		delete this;
		return;
	}

	CPidl cpidl(pidl);

	if( cpidl.IsFolderLink() )
	{
		MSXML2::IXMLDOMNodePtr ptrNode = g_pConfigXML->GetNode(pidl);
		if(ptrNode)
		{
			EnumerateFolderLinks(ptrNode);
			EnumerateFileLinks(ptrNode);
			TCHAR szPath[MAX_PATH] = {0};
			CConfigXML::GetNodeAttribute(ptrNode, TEXT("path"), szPath, MAX_PATH);
			EnumerateDirectory(szPath, dwFlags);
		}
	}
	else if( cpidl.IsSubFolder() )
	{
		TCHAR szPath[MAX_PATH] = {0};
		if( cpidl.GetFSPath(szPath, MAX_PATH) > 0)
			EnumerateDirectory(szPath, dwFlags);
	}
	else
	{
		MSXML2::IXMLDOMNodePtr node = g_pConfigXML->GetNode(pidl);
		if(node != NULL)
		{
			if(dwFlags & SHCONTF_FOLDERS)
				EnumerateFolders( node );
			if(dwFlags & SHCONTF_NONFOLDERS)
				EnumerateFiles( node );
		}
	}

	m_ObjRefCount = 1;
	
	gAddRef(CENUMIDLIST);
}

CEnumIDList::~CEnumIDList()
{
	DeleteList();
	
	if(m_pMalloc)
		m_pMalloc->Release();

	gRelease(CENUMIDLIST);
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

STDMETHODIMP CEnumIDList::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;
	
	//IUnknown
	if(IsEqualIID(riid, IID_IUnknown))
		*ppReturn = this;
	
	//IEnumIDList
	else if(IsEqualIID(riid, IID_IEnumIDList))
		*ppReturn = (IEnumIDList*)this;
	
	if(*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CEnumIDList::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CEnumIDList::Release()
{
	if(--m_ObjRefCount == 0)
	{
		delete this;
		return 0;
	}
	
	return m_ObjRefCount;
}

///////////////////////////////////////////////////////////////////////////
//
// IEnumIDList Implementation
//

/**************************************************************************
IEnumIDList::Clone
	Creates a new item enumeration object with the same contents
	and state as the current one.

Parameters
	ppenum	Address of a pointer to the new enumeration object.
			The calling application must eventually free the new object
			by calling its Release member function.

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CEnumIDList::Clone(LPENUMIDLIST *ppEnum)
{
	return E_NOTIMPL;
}

/**************************************************************************
IEnumIDList::Next
	Retrieves the specified number of item identifiers in the enumeration
	sequence and advances the current position by the number of items retrieved.

Parameters
	celt [in]		Number of elements in the array referenced by the
					rgelt parameter.
	rgelt [out]		Address of a pointer to an array of ITEMIDLIST
					pointers that receive the item identifiers.
					The implementation must allocate these item
					identifiers using the Shell's allocator (retrieved
					by the SHGetMalloc function). The calling application
					is responsible for freeing the item identifiers using
					the Shell's allocator.
					The ITEMIDLIST structures returned in the array are
					relative to the IShellFolder being enumerated.
	pceltFetched [out]	Pointer to a value that receives a count of the
						item identifiers actually returned in rgelt.
						The count can be smaller than the value specified
						in the celt parameter. This parameter can be NULL
						on entry only if celt is one, because in that case
						the method can only retrieve one (S_OK) or zero
						(S_FALSE) items.

Return Value
	Returns S_OK if the method successfully retrieved the requested celt
	elements. This method only returns S_OK if the full count of requested
	items are successfully retrieved.
	S_FALSE indicates that more items were requested than remained in the
	enumeration. The value pointed to by the pceltFetched parameter
	specifies the actual number of items retrieved. Note that the value
	will be 0 if there are no more items to retrieve.
	Returns an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CEnumIDList::Next(DWORD dwElements,
							   LPITEMIDLIST apidl[],
							   LPDWORD pdwFetched)
{
	DWORD    dwIndex;
	HRESULT  hr = S_OK;
	
	if(dwElements > 1 && !pdwFetched)
		return E_INVALIDARG;
	
	for(dwIndex = 0; dwIndex < dwElements; dwIndex++)
	{
		//is this the last item in the list?
		if(!m_pCurrent)
		{
			hr =  S_FALSE;
			break;
		}
		
		apidl[dwIndex] = g_pPidlMgr->Copy(m_pCurrent->pidl);
		
		m_pCurrent = m_pCurrent->pNext;
	}
	
	if(pdwFetched)
		*pdwFetched = dwIndex;
	
	return hr;
}

/**************************************************************************
IEnumIDList::Reset
	Returns to the beginning of the enumeration sequence.

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CEnumIDList::Reset(void)
{
	m_pCurrent = m_pFirst;
	
	return S_OK;
}

/**************************************************************************
IEnumIDList::Skip
	Skips over the specified number of elements in the enumeration sequence.

Parameters
	celt	Number of item identifiers to skip.

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise. 
**************************************************************************/
STDMETHODIMP CEnumIDList::Skip(DWORD dwSkip)
{
	DWORD    dwIndex;
	HRESULT  hr = S_OK;

	for(dwIndex = 0; dwIndex < dwSkip; dwIndex++)
	{
		//is this the last item in the list?
		if(!m_pCurrent)
		{
			hr = S_FALSE;
			break;
		}
		
		m_pCurrent = m_pCurrent->pNext;
	}
	
	return hr;
}

///////////////////////////////////////////////////////////////////////////
//
// Private functions
//

BOOL CEnumIDList::AddToEnumList(LPITEMIDLIST pidl)
{
	LPENUMLIST  pNew;
	
	pNew = (LPENUMLIST)m_pMalloc->Alloc(sizeof(ENUMLIST));
	
	if(pNew)
	{
		//set the next pointer
		pNew->pNext = NULL;
		pNew->pidl = pidl;
		
		//is this the first item in the list?
		if(!m_pFirst)
		{
			m_pFirst = pNew;
			m_pCurrent = m_pFirst;
		}
		
		if(m_pLast)
		{
			//add the new item to the end of the list
			m_pLast->pNext = pNew;
		}
		
		//update the last item pointer
		m_pLast = pNew;
		
		return TRUE;
	}
	
	return FALSE;
}

BOOL CEnumIDList::DeleteList(void)
{
	LPENUMLIST  pDelete;
	
	while(m_pFirst)
	{
		pDelete = m_pFirst;
		m_pFirst = pDelete->pNext;
		
		//free the pidl
		g_pPidlMgr->Delete(pDelete->pidl);
		
		//free the list item
		m_pMalloc->Free(pDelete);
	}
	
	m_pFirst = m_pLast = m_pCurrent = NULL;
	
	return TRUE;
}

BOOL CEnumIDList::EnumerateFiles(MSXML2::IXMLDOMNodePtr node)
{
	EnumerateFileLinks( node );

	LPITEMIDLIST pidl;
	IGoogleDesktopQueryResultSet *pResults;
	IGoogleDesktopQueryResultItemPtr spItem;
	UINT ranking = RELEVANCE_RANKING;

	m_iFileCount = 0;

	PIDLDATA data;
	memset(&data, 0, sizeof(PIDLDATA));
	if( !CConfigXML::GetFolderInfo(&data, node) )
		return FALSE;

	BOOL bHasSubFolders = (m_iFolderCount > 0);
	pResults = CGoogleDS::Query(&data, bHasSubFolders);

	BOOL bFilesOnly = (0 == lstrcmpi(data.searchData.szCategory, "file"));

	if(NULL == pResults)
		return TRUE;

	UINT maxResults = data.searchData.maxResults;

	while(NULL != (spItem = pResults->Next()))
	{
		pidl = g_pPidlMgr->CreateFileFromSearch(spItem);
		
		if(!pidl)
			return FALSE;

		LPPIDLDATA pData = CPidlManager::GetDataPointer(pidl);
		if(!bFilesOnly || pData->fileData.pidlFS)
		{
			if(!AddToEnumList(pidl))
				return FALSE;

			m_iFileCount++;

			if((maxResults > 0) && (m_iFileCount >= maxResults))
				break;
		}
		else
			g_pPidlMgr->Delete(pidl);
	}

	return TRUE;
}

BOOL CEnumIDList::EnumerateFolders(MSXML2::IXMLDOMNodePtr node)
{
	LPITEMIDLIST				pidl;
	MSXML2::IXMLDOMNodePtr		child;
	MSXML2::IXMLDOMNodeListPtr	list;

	list = g_pConfigXML->GetFolders(node);

	for(long i=0; i<list->Getlength(); i++)
	{
		child = list->Getitem(i);

		pidl = g_pPidlMgr->CreateFolder(child);

		if(!pidl)
			return FALSE;

		if(!AddToEnumList(pidl))
			return FALSE;

		m_iFolderCount++;

		child = child->nextSibling;
	}

	EnumerateFolderLinks( node );

	return TRUE;
}

BOOL CEnumIDList::EnumerateFolderLinks(MSXML2::IXMLDOMNodePtr node)
{
	long						index;
	LPITEMIDLIST				pidl;
	MSXML2::IXMLDOMNodePtr		child;
	MSXML2::IXMLDOMNodeListPtr	list;
	
	list = g_pConfigXML->GetFolderLinks(node);

	for(index=0; index<list->Getlength(); index++)
	{
		child = list->Getitem(index);

		pidl = g_pPidlMgr->CreateFolderLink(child);

		if(!pidl || !AddToEnumList(pidl))
			return FALSE;

		m_iFolderCount++;

		child = child->nextSibling;
	}

	return TRUE;
}

BOOL CEnumIDList::EnumerateFileLinks(MSXML2::IXMLDOMNodePtr node)
{
	long						index;
	LPITEMIDLIST				pidl;
	MSXML2::IXMLDOMNodePtr		child;
	MSXML2::IXMLDOMNodeListPtr	list;
	
	list = g_pConfigXML->GetFileLinks(node);

	for(index=0; index<list->Getlength(); index++)
	{
		child = list->Getitem(index);

		pidl = g_pPidlMgr->CreateFileFromNode(child);

		if(!pidl || !AddToEnumList(pidl))
			return FALSE;

		m_iFolderCount++;

		child = child->nextSibling;
	}

	return TRUE;
}

BOOL CEnumIDList::EnumerateDirectory(LPCTSTR lpszPath, DWORD dwFlags)
{
	WIN32_FIND_DATA	ffd = {0};
	HANDLE			hFind = INVALID_HANDLE_VALUE;
	DWORD			dwError;
	TCHAR			DirSpec[MAX_PATH] = {0};
	BOOL			bResult = FALSE;
	LPITEMIDLIST	pidlTmp;
	TCHAR			szFullPath[MAX_PATH] = {0};
	LPPIDLDATA		pData = NULL;

	// Prepare string for use with FindFile functions.  First, 
	// copy the string to a buffer, then append '\*' to the 
	// directory name.
	_tcsncpy(DirSpec, lpszPath, _tcslen(lpszPath)+1);
	_tcsncat(DirSpec, TEXT("\\*"), 2*sizeof(TCHAR));
	
	hFind = ::FindFirstFile(DirSpec, &ffd);
	
	if(INVALID_HANDLE_VALUE != hFind) 
	{
		do {
			pidlTmp = NULL;
			_tcsnset(szFullPath, 0, MAX_PATH);
			_tcscpy(szFullPath, lpszPath);
			_tcscat(szFullPath, "\\");
			_tcscat(szFullPath, ffd.cFileName);
			bool bFolder = ((FILE_ATTRIBUTE_DIRECTORY & ffd.dwFileAttributes) > 0);
			if( bFolder )
			{
				if(dwFlags & SHCONTF_FOLDERS)
					pidlTmp = g_pPidlMgr->CreateSubFolder(szFullPath);
			}
			else
			{
				if(dwFlags & SHCONTF_NONFOLDERS)
					pidlTmp = g_pPidlMgr->CreateFileFromPath(szFullPath);
			}
			if(NULL != pidlTmp)
			{
				pData = CPidlManager::GetDataPointer(pidlTmp);
				if(pData->fileData.pidlFS)
				{
					if( !AddToEnumList(pidlTmp) )
						break;
					if( bFolder )
						m_iFolderCount++;
					else
						m_iFileCount++;
				}
				else
				{
					g_pPidlMgr->Delete(pidlTmp);
				}
			}
		} while(::FindNextFile(hFind, &ffd) != 0);
		
		dwError = GetLastError();
		::FindClose(hFind);
		bResult = (ERROR_NO_MORE_FILES == dwError);
	}

	return bResult;
}
