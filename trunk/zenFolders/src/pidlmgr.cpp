// fldrmgr.cpp: implementation of the CPidlManager class.
//
//////////////////////////////////////////////////////////////////////

#include <crtdbg.h>
#include "pidlmgr.h"
#include "cfgxml.h"
#include "util/settings.h"
#include "util/string.h"
#include "zenfolders.h"
//#include "util/shellnse.h"
//#include "guid.h"


#define ARRAYSIZE(a)	(sizeof(a)/sizeof(a[0]))


extern HINSTANCE		g_hInst;
extern LPCONFIGXML		g_pConfigXML;


CPidlManager::CPidlManager()
{
	if(FAILED(::SHGetMalloc(&m_pMalloc)))
	{
		delete this;
		return;
	}

	::SHGetDesktopFolder( &m_psfDesktop );
	
//	gAddRef(CPIDLMANAGER);
}

CPidlManager::~CPidlManager()
{
	if(m_pMalloc)
		m_pMalloc->Release();

	if(m_psfDesktop)
		m_psfDesktop->Release();

//	gRelease(CPIDLMANAGER);
}

void CPidlManager::Delete(LPITEMIDLIST pidl)
{
	m_pMalloc->Free(pidl);
}

LPITEMIDLIST CPidlManager::GetNextItem(LPCITEMIDLIST pidl)
{
	if(pidl)
		return (LPITEMIDLIST)(((LPBYTE)pidl) + pidl->mkid.cb);
	else
		return (NULL);
}

UINT CPidlManager::GetSize(LPCITEMIDLIST pidl)
{
	UINT cbTotal = 0;
	LPITEMIDLIST pidlTemp = (LPITEMIDLIST) pidl;
	
	if(pidlTemp)
	{
		while(pidlTemp->mkid.cb)
		{
			cbTotal += pidlTemp->mkid.cb;
			pidlTemp = GetNextItem(pidlTemp);
		}  
		
		//add the size of the NULL terminating ITEMIDLIST
		cbTotal += sizeof(ITEMIDLIST);
	}
	
	return (cbTotal);
}

LPITEMIDLIST CPidlManager::GetLastItem(LPCITEMIDLIST pidl)
{
	LPITEMIDLIST pidlLast = NULL;
	
	//get the PIDL of the last item in the list
	if(pidl)
	{
		while(pidl->mkid.cb)
		{
			pidlLast = (LPITEMIDLIST)pidl;
			pidl = GetNextItem(pidl);
		}  
	}
	
	return pidlLast;
}

LPITEMIDLIST CPidlManager::Copy(LPCITEMIDLIST pidlSource)
{
	LPITEMIDLIST pidlTarget = NULL;
	UINT cbSource = 0;
	
	if(NULL == pidlSource)
		return (NULL);
	
	// Allocate the new pidl
	cbSource = GetSize(pidlSource);
	pidlTarget = (LPITEMIDLIST)m_pMalloc->Alloc(cbSource);
	if(!pidlTarget)
		return (NULL);
	
	// Copy the source to the target
	CopyMemory(pidlTarget, pidlSource, cbSource);

	return pidlTarget;
}

LPPIDLDATA CPidlManager::GetDataPointer(LPCITEMIDLIST pidl)
{
	return pidl ? (LPPIDLDATA)(pidl->mkid.abID) : NULL;
}

bool CPidlManager::IsOurPidl(LPCITEMIDLIST pidl)
{
	return (pidl->mkid.cb == (sizeof(ITEMIDLIST) + sizeof(PIDLDATA)));
}

LPITEMIDLIST CPidlManager::Concatenate(LPCITEMIDLIST pidl1,
									   LPCITEMIDLIST pidl2)
{
	LPITEMIDLIST   pidlNew;
	UINT           cb1 = 0, cb2 = 0;
	
	//are both of these NULL?
	if(!pidl1 && !pidl2)
		return NULL;
	
	//if pidl1 is NULL, just return a copy of pidl2
	if(!pidl1)
	{
		pidlNew = Copy(pidl2);
		return pidlNew;
	}
	
	//if pidl2 is NULL, just return a copy of pidl1
	if(!pidl2)
	{
		pidlNew = Copy(pidl1);
		return pidlNew;
	}
	
	cb1 = GetSize(pidl1) - sizeof(ITEMIDLIST);
	
	cb2 = GetSize(pidl2);
	
	//create the new PIDL
	pidlNew = (LPITEMIDLIST)m_pMalloc->Alloc(cb1 + cb2);

	if(pidlNew)
	{
		//copy the first PIDL
		CopyMemory(pidlNew, pidl1, cb1);
		
		//copy the second PIDL
		CopyMemory(((LPBYTE)pidlNew) + cb1, pidl2, cb2);
	}
	
	return pidlNew;
}

bool CPidlManager::SetTitle(IGoogleDesktopQueryResultItem *pItem, LPPIDLDATA pData)
{
#ifdef _DEBUG
	return false;
#endif

	_variant_t val;
	_bstr_t title(::SysAllocString(L"title"));
	try
	{
		val = pItem->GetProperty(title);
	}
	catch(...)
	{
		// Why isn't this enough in the debug version?
		return false;
	}

	int len = sizeof(pData->szName) / sizeof(TCHAR);
	CString::WideCharToLocal(pData->szName, val.bstrVal, len);
	return (pData->szName[1] != ':');
}

LPITEMIDLIST CPidlManager::CreateFileFromSearch(IGoogleDesktopQueryResultItem *pItem)
{
	UINT		len;
	BOOL		bExtractFilename = FALSE;
	PIDLDATA	data;
	_variant_t	val;
	LPFILEDATA	pFileData = &data.fileData;

	memset(&data, 0, sizeof(PIDLDATA));

	data.type = PT_FILE;

	_bstr_t uri(::SysAllocString(L"uri"));
	val = pItem->GetProperty(uri);
	len = sizeof(pFileData->szPath)/sizeof(TCHAR);
	CString::WideCharToLocal(pFileData->szPath, val.bstrVal, len);

	if(!SetTitle(pItem, &data) || (0 == lstrcmp(pFileData->szPath, data.szName)))
	{
		LPCTSTR pszName = strrchr(pFileData->szPath, '\\');
		if(NULL == pszName)
			pszName = pFileData->szPath;
		else
			pszName++;
		memset(data.szName, 0, sizeof(data.szName));
		lstrcpyn(data.szName, pszName, ARRAYSIZE(data.szName));
	}

	pFileData->pidlFS = GetFSPidl(pFileData->szPath);

	return Create(PT_FILE, (LPVOID)&data, 0);
}

LPITEMIDLIST CPidlManager::CreateFileFromNode(MSXML2::IXMLDOMNodePtr node)
{
	TCHAR szPath[MAX_PATH] = {0};
	CConfigXML::GetNodeAttribute(node, TEXT("path"), szPath, MAX_PATH);

	return CreateFileFromPath(szPath);
}

LPITEMIDLIST CPidlManager::CreateFileFromPath(LPCTSTR lpszPath)
{
	BOOL		bExtractFilename = FALSE;
	PIDLDATA	data;
	LPFILEDATA	pFileData = &data.fileData;

	memset(&data, 0, sizeof(PIDLDATA));

	data.type = PT_FILE;

	CString::WideCharToLocal(pFileData->szPath, _bstr_t(lpszPath),  _tcslen(lpszPath));

	LPCTSTR pszName = strrchr(pFileData->szPath, '\\');
	if(NULL == pszName)
		pszName = pFileData->szPath;
	else
		pszName++;
	memset(data.szName, 0, sizeof(data.szName));
	lstrcpyn(data.szName, pszName, ARRAYSIZE(data.szName));

	pFileData->pidlFS = GetFSPidl(pFileData->szPath);

	return Create(PT_FILE, (LPVOID)&data, 0);
}

LPITEMIDLIST CPidlManager::CreateFolder(MSXML2::IXMLDOMNodePtr node)
{
	return Create(PT_FOLDER, (LPVOID)node, 0);
}

LPITEMIDLIST CPidlManager::CreateSubFolder(LPCTSTR lpszPath)
{
	BOOL		bExtractFilename = FALSE;
	PIDLDATA	data;
	LPFILEDATA	pFileData = &data.fileData;

	memset(&data, 0, sizeof(PIDLDATA));

	data.type = PT_SUB_FOLDER;

	CString::WideCharToLocal(pFileData->szPath, _bstr_t(lpszPath),  _tcslen(lpszPath));

	LPCTSTR pszName = strrchr(pFileData->szPath, '\\');
	if(NULL == pszName)
		pszName = pFileData->szPath;
	else
		pszName++;
	memset(data.szName, 0, sizeof(data.szName));
	lstrcpyn(data.szName, pszName, ARRAYSIZE(data.szName));

	pFileData->pidlFS = GetFSPidl(pFileData->szPath);

	if(NULL != pFileData->pidlFS)
		return Create(PT_SUB_FOLDER, (LPVOID)&data, 0);
	else
		return NULL;
}

LPITEMIDLIST CPidlManager::CreateFolderLink(MSXML2::IXMLDOMNodePtr node)
{
	return Create(PT_FOLDER_LINK, (LPVOID)node, 0);
}

LPITEMIDLIST CPidlManager::Create(PIDLTYPE type, LPVOID pIn, USHORT uInSize)
{
	LPITEMIDLIST	pidlOut = NULL;
	USHORT			uSize;
	USHORT			uSizeOut;
	LPITEMIDLIST	pidlTemp;
	LPPIDLDATA		pData;
	LPPIDLDATA		pDataIn;

	uSize = sizeof(ITEMIDLIST) + sizeof(PIDLDATA);
	uSizeOut = uSize + sizeof(ITEMIDLIST);

	// Allocate the memory, adding an additional ITEMIDLIST
	// for the NULL terminating  ID List.
	pidlOut = (LPITEMIDLIST)m_pMalloc->Alloc( uSizeOut );
	
	if(pidlOut)
	{
		memset(pidlOut, 0, uSizeOut);
		pidlTemp = pidlOut;
		pidlTemp->mkid.cb = uSize;
		pData = GetDataPointer(pidlTemp);
		memset(pData->szName, 0, sizeof(pData->szName));
		pData->type = type;

		switch(type)
		{
		case PT_FOLDER:
			CConfigXML::GetFolderInfo(pData, (MSXML2::IXMLDOMNode*)pIn);
			break;

		case PT_FOLDER_LINK:
			CConfigXML::GetFolderLinkInfo(pData, (MSXML2::IXMLDOMNode*)pIn);
			break;

		case PT_FILE:
		case PT_SUB_FOLDER:
			pDataIn = (LPPIDLDATA)pIn;
			lstrcpyn(pData->szName, pDataIn->szName, sizeof(pData->szName));
			memcpy(&pData->fileData, &pDataIn->fileData, sizeof(pData->fileData));
			break;
		}
		
		//set the NULL terminator to 0
		pidlTemp = GetNextItem(pidlTemp);
		memset(pidlTemp, 0, sizeof(ITEMIDLIST));
	}

	return pidlOut;
}

bool CPidlManager::HasSubFolders(LPCITEMIDLIST pidl)
{
	if( IsFile(pidl) )
		return false;

	if( IsFolder(pidl) )
	{
		return g_pConfigXML->HasSubFolders( pidl );
	}
	else
	{
		int check_sub_folders;
		return true;
	}
/*
	else if( IsSubFolder(pidlRel) )
	{
	}
	else if( IsFolderLink(pidlRel) )
	{
	}
*/
}

DWORD CPidlManager::GetItemName(LPCITEMIDLIST pidl,
								LPTSTR lpszText,
								USHORT uSize)
{
	LPPIDLDATA  pData = GetDataPointer(pidl);
	lstrcpyn(lpszText, pData->szName, uSize);
	return lstrlen(lpszText);
}

HRESULT CPidlManager::CompareIDs(LPCITEMIDLIST pidl1,
								 LPCITEMIDLIST pidl2)
{
	LPCITEMIDLIST pidlTemp1 = pidl1, pidlTemp2 = pidl2;
	TCHAR szString1[MAX_PATH] = TEXT("");
	TCHAR szString2[MAX_PATH] = TEXT("");
	
	while(true)
	{
		if(pidlTemp1->mkid.cb == NULL)
			return ((pidlTemp2->mkid.cb == NULL) ? 0 : -1);

		if(pidlTemp2->mkid.cb == NULL)
			return ((pidlTemp1->mkid.cb == NULL) ? 0 : 1);
		
		if(IsFile(pidlTemp1) != IsFile(pidlTemp2))
		{
			return (IsFile(pidlTemp1) ? 1 : -1);
		}
		else 
		{
			memset(szString1, 0, sizeof(szString1));
			GetItemName(pidlTemp1, szString1, sizeof(szString1));

			memset(szString2, 0, sizeof(szString2));
			GetItemName(pidlTemp2, szString2, sizeof(szString2));
			
			//now compare the value strings
			int i = lstrcmpi(szString1, szString2);
			if(i != 0)
				return i;
		}
		pidlTemp1 = GetNextItem(pidlTemp1);
		pidlTemp2 = GetNextItem(pidlTemp2);
	}

	return 0;
}

bool CPidlManager::Equal(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	if((NULL == pidl1) && (NULL == pidl2))
		return true;

	if((NULL == pidl1) || (NULL == pidl2))
		return false;

	LPCITEMIDLIST pidlRel1 = GetLastItem(pidl1);
	LPCITEMIDLIST pidlRel2 = GetLastItem(pidl2);

	LPPIDLDATA pData1 = GetDataPointer(pidlRel1);
	LPPIDLDATA pData2 = GetDataPointer(pidlRel2);

	if(pData1->type != pData2->type)
		return false;

	if(0 != lstrcmpi(pData1->szName, pData2->szName))
		return false;

	switch(pData1->type)
	{
	case PT_FILE:
		return (0 == lstrcmpi(pData1->fileData.szPath, pData2->fileData.szPath));

	case PT_FOLDER:
		{
			LPSEARCHDATA pFolder1 = &pData1->searchData;
			LPSEARCHDATA pFolder2 = &pData2->searchData;

			if(pFolder1->maxResults != pFolder2->maxResults)
				return false;

			if(pFolder1->ranking != pFolder2->ranking)
				return false;

			if(0 != lstrcmpi(pFolder1->szCategory, pFolder2->szCategory))
				return false;

			if(0 != lstrcmpi(pFolder1->szQuery, pFolder2->szQuery))
				return false;
		}

	default:
		_RPTF1(_CRT_WARN, "pData1->type unknown (%d)!\n", pData1->type);
	}

	return true;
}

DWORD CPidlManager::GetPidlPath(LPCITEMIDLIST pidl,
								LPTSTR lpszOut,
								DWORD dwOutSize)
{
	if(!pidl)
		return 0;
	
	LPITEMIDLIST   pidlTemp;
	DWORD          dwCopied = 0;
	TCHAR          szText[MAX_PATH];

	pidlTemp = (LPITEMIDLIST)pidl;
	
	//if this is NULL, return the required size of the buffer
	if(!lpszOut)
	{
		while(pidlTemp->mkid.cb)
		{
			LPPIDLDATA  pData = GetDataPointer(pidlTemp);
			
			//add the length of this item plus one for the backslash
			if((pData->type == PT_FOLDER) || (pData->type == PT_FILE))
				dwCopied += lstrlen(pData->fileData.szPath) + 1;
			
			pidlTemp = GetNextItem(pidlTemp);
		}
		
		//add one for the NULL terminator
		return dwCopied + 1;
	}
	
	*lpszOut = 0;
	
	bool bBreak = false;
	while(pidlTemp && !bBreak && pidlTemp->mkid.cb && (dwCopied < dwOutSize))
	{
		LPPIDLDATA  pData = GetDataPointer(pidlTemp);

		switch(pData->type)
		{
		case PT_FILE:
			bBreak = true;
			break;

		case PT_FOLDER:
		case PT_FOLDER_LINK:
			dwCopied += GetItemName(pidlTemp, szText, sizeof(szText)/sizeof(TCHAR));
			lstrcat(lpszOut, szText);
			break;

		default:
			{
				TCHAR szName[MAX_PATH] = {0};
				dwCopied += CSettings::GetRootName(pidlTemp, szName, sizeof(szName)/sizeof(TCHAR));
				lstrcat(lpszOut, szName);
			}
		}

		lstrcat(lpszOut, TEXT("\\"));
		dwCopied += 1;
		
		pidlTemp = GetNextItem(pidlTemp);
	}
	
	//remove the last backslash if necessary
	if(dwCopied)
	{
		if(*(lpszOut + lstrlen(lpszOut) - 1) == '\\')
		{
			*(lpszOut + lstrlen(lpszOut) - 1) = 0;
			dwCopied--;
		}
	}
	
	return dwCopied;
}

PIDLTYPE CPidlManager::GetType(LPCITEMIDLIST pidl)
{
	if(NULL == pidl)
		return PT_UNKNOWN;

	LPITEMIDLIST pidlLast = GetLastItem(pidl);

	LPPIDLDATA  pData = GetDataPointer(pidlLast);
	if(NULL == pData)
		return PT_UNKNOWN;

	return pData->type;
}

bool CPidlManager::IsRoot(LPCITEMIDLIST pidl)
{
	if(NULL == pidl)
		return false;
	
	return (
		!IsFile(pidl) &&
		!IsFolder(pidl) &&
		!IsFolderLink(pidl) &&
		!IsSubFolder(pidl)
		);
}

bool CPidlManager::IsFile(LPCITEMIDLIST pidl)
{
	return (PT_FILE == GetType(pidl));
}

bool CPidlManager::IsFolder(LPCITEMIDLIST pidl)
{
	return (PT_FOLDER == GetType(pidl));
}

bool CPidlManager::IsFolderLink(LPCITEMIDLIST pidl)
{
	return (PT_FOLDER_LINK == GetType(pidl));
}

bool CPidlManager::IsSubFolder(LPCITEMIDLIST pidl)
{
	return (PT_SUB_FOLDER == GetType(pidl));
}

#ifdef _DEBUG
void CPidlManager::dbgTracePidlPath(LPCTSTR text, LPCITEMIDLIST pidl)
{
	TCHAR szPath[MAX_PATH] = TEXT("NULL");
	UINT len = sizeof(szPath)/sizeof(TCHAR);
	if(pidl)
		GetPidlPath(pidl, szPath, len);
	_RPTF1(_CRT_WARN, text, szPath);
}

void CPidlManager::dbgTracePidlData(LPCTSTR text, LPCITEMIDLIST pidl)
{
	TCHAR szOutput[MAX_PATH] = TEXT("NULL");
	UINT len = sizeof(szOutput)/sizeof(TCHAR);
	if(pidl)
	{
		LPPIDLDATA pData = GetDataPointer(pidl);
		if( IsFile(pidl) )
		{
			wsprintf(szOutput, "name: %s, path: %s",
				pData->szName, pData->fileData.szPath);
		}
		else
		{
			wsprintf(szOutput, "name: %s, query: %s, category: %s, ranking: %d, max: %d",
				pData->szName,
				pData->searchData.szQuery,
				pData->searchData.szCategory,
				pData->searchData.ranking,
				pData->searchData.maxResults);
		}
	}
	_RPTF1(_CRT_WARN, text, szOutput);
}
#endif	// _DEBUG

LPITEMIDLIST CPidlManager::GetFSPidl(LPCTSTR lpszPath)
{
	HRESULT hr;
	LPITEMIDLIST pidlFS = NULL;
	OLECHAR *olePath = NULL;
	int len = lstrlen(lpszPath);

	olePath = (OLECHAR*)calloc(len+1, sizeof(OLECHAR));

	::MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED,
		lpszPath,
		-1,
		olePath,
		len+1);	

	hr = m_psfDesktop->ParseDisplayName(
		NULL,
		0,
		olePath,
		NULL,
		&pidlFS,
		NULL);

	free(olePath);

	return pidlFS;
}

LPITEMIDLIST CPidlManager::GetFSPidl(LPCITEMIDLIST pidl)
{
	CPidl cpidl(pidl);
	LPPIDLDATA pData = cpidl.GetData();
	return GetFSPidl(pData->fileData.szPath);
}
