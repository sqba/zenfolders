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

extern HINSTANCE	g_hInst;
extern LPCONFIGXML	g_pConfigXML;


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

/*
	LPPIDLDATA pData1 = GetDataPointer(pidlTarget);
	LPPIDLDATA pData2 = GetDataPointer(pidlSource);

	pData1->maxResults = pData2->maxResults;
	pData1->type = pData2->type;
	lstrcpy(pData1->szName, pData2->szName);
	if(IsFile(pidlSource))
	{
		lstrcpy(pData1->szPath, pData2->szPath);
	}
	else
	{
		pData1->folderData.maxResults = pData2->folderData.maxResults;
		pData1->folderData.pNode = pData2->folderData.pNode;
		pData1->folderData.ranking = pData2->folderData.ranking;
		lstrcpy(pData1->folderData.szCategory, pData2->folderData.szCategory);
		lstrcpy(pData1->folderData.szQuery, pData2->folderData.szQuery);
	}
*/
	return pidlTarget;
}

LPPIDLDATA CPidlManager::GetDataPointer(LPCITEMIDLIST pidl)
{
	if(!pidl)
		return NULL;

//	if(pidl->mkid.cb != (sizeof(ITEMIDLIST) + sizeof(PIDLDATA)))
//		return NULL;
	
	return (LPPIDLDATA)(pidl->mkid.abID);
}

BOOL CPidlManager::IsOurPidl(LPCITEMIDLIST pidl)
{
	//root: pidl->mkid.cb = 0
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
//		LPITEMIDLIST next = GetNextItem(pidlNew);
//		CopyMemory(next, pidl2, cb2);

//		LPPIDLDATA pData1 = GetDataPointer(pidl1);// debug
//		LPPIDLDATA pData2 = GetDataPointer(pidl2);// debug
//		LPPIDLDATA pData3 = GetDataPointer(pidlNew);// debug
//		LPPIDLDATA pData4 = GetDataPointer(next);// debug
	}
	
	return pidlNew;
}

LPITEMIDLIST CPidlManager::CreateFile(IGoogleDesktopQueryResultItem *pItem)
{
	UINT len;
	BOOL bExtractFilename = FALSE;
	PIDLDATA data;
	LPFILEDATA pFileData = &data.fileData;
	memset(&data, 0, sizeof(PIDLDATA));

	data.type = PT_FILE;

	_variant_t val;

	_bstr_t uri(::SysAllocString(L"uri"));
	val = pItem->GetProperty(uri);
	len = sizeof(pFileData->szPath)/sizeof(TCHAR);
	CString::WideCharToLocal(pFileData->szPath, val.bstrVal, len);

	try
	{
		_bstr_t title(::SysAllocString(L"title"));
		val = pItem->GetProperty(title);
		len = sizeof(data.szName)/sizeof(TCHAR);
		CString::WideCharToLocal(data.szName, val.bstrVal, len);
		if(data.szName[1] == ':')
			bExtractFilename = TRUE;
	}
	catch(...)
	{
		bExtractFilename = TRUE;
	}

	if(bExtractFilename || (0 == lstrcmp(pFileData->szPath, data.szName)))
	{
		LPCTSTR pszName = strrchr(pFileData->szPath, '\\');
		if(NULL == pszName)
			pszName = pFileData->szPath;
		else
			pszName++;
		memset(data.szName, 0, sizeof(data.szName));
		CopyMemory(data.szName, pszName, lstrlen(pszName));
	}

	pFileData->pidlFS = GetFSPidl(pFileData->szPath);

	return Create(PT_FILE, (LPVOID)&data, 0);
}

LPITEMIDLIST CPidlManager::CreateFolder(MSXML2::IXMLDOMNodePtr node)
{
	return Create(PT_FOLDER, (LPVOID)node, 0);
}

LPITEMIDLIST CPidlManager::Create(PIDLTYPE type, LPVOID pIn, USHORT uInSize)
{
	LPITEMIDLIST	pidlOut;
	USHORT			uSize;

	pidlOut = NULL;

	uSize = sizeof(ITEMIDLIST) + sizeof(PIDLDATA);

	// Allocate the memory, adding an additional ITEMIDLIST
	// for the NULL terminating  ID List.
	pidlOut = (LPITEMIDLIST)m_pMalloc->Alloc(uSize + sizeof(ITEMIDLIST));
	
	if(pidlOut)
	{
		memset(pidlOut, 0, uSize + sizeof(ITEMIDLIST));

		LPITEMIDLIST   pidlTemp = pidlOut;
		LPPIDLDATA     pData;
		
		pidlTemp->mkid.cb = uSize;
		
		pData = GetDataPointer(pidlTemp);

		memset(pData->szName, 0, sizeof(pData->szName));

		pData->type = type;
		switch(type)
		{
		case PT_FOLDER:
			{
				CConfigXML::GetFolderInfo(pData, (MSXML2::IXMLDOMNode*)pIn);
			}
			break;
			
		case PT_FILE:
			{
				LPPIDLDATA pDataIn = (LPPIDLDATA)pIn;
				//CopyMemory(pData, pDataIn, sizeof(LPPIDLDATA));
				CopyMemory(pData->szName, pDataIn->szName, sizeof(pData->szName));
				CopyMemory(&pData->fileData, &pDataIn->fileData, sizeof(pData->fileData));
				/*LPCTSTR pszFilePath = (LPCTSTR)pIn;
				LPCTSTR pszName = strrchr(pszFilePath, '\\');
				if(NULL == pszName)
					pszName = pszFilePath;
				else
					pszName++;
				UINT uNameSize = lstrlen(pszName);

				CopyMemory(pData->szName, pszName, uNameSize);

				UINT size = sizeof(pData->szPath)/sizeof(TCHAR);
				memset(pData->szPath, 0, size);
				CopyMemory(pData->szPath, pIn, uInSize);*/
			}
			break;
		}
		
		//set the NULL terminator to 0
		pidlTemp = GetNextItem(pidlTemp);
		memset(pidlTemp, 0, sizeof(ITEMIDLIST));
		//pidlTemp->mkid.cb = 0;
		//pidlTemp->mkid.abID[0] = 0;
	}

	return pidlOut;
}

BOOL CPidlManager::HasChildNodes(LPCITEMIDLIST pidl)
{
	if( IsFile(pidl) )
		return FALSE;

	try
	{
		MSXML2::IXMLDOMNodePtr ptrNode = g_pConfigXML->GetNode(pidl);
		LPPIDLDATA pData = GetDataPointer(pidl);
		if(NULL == ptrNode)
			return FALSE;
		VARIANT_BOOL hasChild = ptrNode->hasChildNodes();
		return hasChild;
	}
	catch(...)
	{
		_RPTF0(_CRT_WARN, "CPidlManager::HasChildNodes exception\n");
		return FALSE;
	}
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
			memset(szString1, 0, MAX_PATH);
			GetItemName(pidlTemp1, szString1, sizeof(szString1));

			memset(szString2, 0, MAX_PATH);
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

BOOL CPidlManager::Equal(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	LPCITEMIDLIST pidlRel1 = GetLastItem(pidl1);
	LPCITEMIDLIST pidlRel2 = GetLastItem(pidl2);

	LPPIDLDATA pData1 = GetDataPointer(pidlRel1);
	LPPIDLDATA pData2 = GetDataPointer(pidlRel2);

	if(pData1->type != pData2->type)
		return FALSE;

	if(0 != lstrcmpi(pData1->szName, pData2->szName))
		return FALSE;

	if(PT_FILE == pData1->type)
	{
		return (0 == lstrcmpi(pData1->fileData.szPath, pData2->fileData.szPath));
	}
	else if(PT_FOLDER == pData1->type)
	{
		LPFOLDERDATA pFolder1 = &pData1->folderData;
		LPFOLDERDATA pFolder2 = &pData2->folderData;

		if(pFolder1->maxResults != pFolder2->maxResults)
			return FALSE;

		if(pFolder1->ranking != pFolder2->ranking)
			return FALSE;

		if(0 != lstrcmpi(pFolder1->szCategory, pFolder2->szCategory))
			return FALSE;

		if(0 != lstrcmpi(pFolder1->szQuery, pFolder2->szQuery))
			return FALSE;
	}
	else
	{
		_RPTF1(_CRT_WARN, "pData1->type unknown (%d)!\n", pData1->type);
	}

	return TRUE;
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
	
	while(pidlTemp->mkid.cb && (dwCopied < dwOutSize))
	{
		LPPIDLDATA  pData = GetDataPointer(pidlTemp);
		//if( IsFile(pidlTemp) )
		if(pData->type == PT_FILE)
			break;

		if(pData->type == PT_FOLDER)
		{
			dwCopied += GetItemName(pidlTemp, szText, sizeof(szText)/sizeof(TCHAR));
			lstrcat(lpszOut, szText);
		}
		else // Root
		{
			TCHAR szName[MAX_PATH] = {0};

			dwCopied += CSettings::GetRootName(pidlTemp, szName, sizeof(szName)/sizeof(TCHAR));

			//lstrcat(lpszOut, TEXT(SHELLEX_NAME));
			lstrcat(lpszOut, szName);
			//dwCopied += (lstrlen(szName));
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

BOOL CPidlManager::IsFile(LPCITEMIDLIST pidl)
{
	if(NULL == pidl)
		return TRUE;

	LPPIDLDATA  pData = GetDataPointer(pidl);
	if(NULL == pData)
		return TRUE;
	
	return (PT_FILE == pData->type);
}

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
				pData->folderData.szQuery,
				pData->folderData.szCategory,
				pData->folderData.ranking,
				pData->folderData.maxResults);
		}
	}
	_RPTF1(_CRT_WARN, text, szOutput);
}

void CPidlManager::dbgTracePidlPath(LPCTSTR text, CPidl *pCpidl)
{
	dbgTracePidlPath(text, pCpidl->GetFull());
}

void CPidlManager::dbgTracePidlData(LPCTSTR text, CPidl *pCpidl)
{
	dbgTracePidlData(text, pCpidl->GetRelative());
}

LPITEMIDLIST CPidlManager::CreateFromPath(LPCITEMIDLIST pidl)
{
	LPITEMIDLIST last = GetLastItem(pidl);
	LPPIDLDATA pData = GetDataPointer(last);
	LPITEMIDLIST result = NULL;
	if(PT_FILE == pData->type)
	{
		LPTSTR lpszPath = pData->fileData.szPath;
		TCHAR *token = strtok(lpszPath, "\\");
        while (token != NULL)
		{
			PIDLDATA data;
			memset(&data, 0, sizeof(PIDLDATA));
			data.type = PT_FILE;
			lstrcpy(data.szName, token);
			lstrcpy(data.fileData.szPath, "CPidlManager::CreateFromPath");
			//_RPTF1(_CRT_WARN, "Created %s\n", data.szName);
			//TRACE_PIDL_DATA("CPidlManager::CreateFromPath(data: %s)\n", tmp);
			if(NULL == result)
				result = Create(PT_FILE, (LPVOID)&data, 0);
			else
			{
				LPITEMIDLIST tmp = Create(PT_FILE, (LPVOID)&data, 0);
				Concatenate(result, tmp);
				Delete(tmp);
			}

			token = strtok(NULL, "\\");
        }
		//TRACE_PIDL_PATH("CPidlManager::CreateFromPath(%s)\n", result);
	}
	else
	{
		//_RPTF0(_CRT_WARN, "PT_FILE == pData->type\n");
	}
	return result;
}

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

//	if(hr == NOERROR)
//		_RPTF1(_CRT_WARN, "psfDesktop->ParseDisplayName(%s)\n", lpszPath);

	free(olePath);

	return pidlFS;
}

LPITEMIDLIST CPidlManager::GetFSPidl(LPCITEMIDLIST pidl)
{
	CPidl cpidl(pidl);
	LPPIDLDATA pData = cpidl.GetData();
	return GetFSPidl(pData->fileData.szPath);
}
