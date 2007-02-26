#include "dragdrop.h"
#include "../shlfldr.h"
#include "string.h"


BOOL CDragAndDrop::GetTextFromSTRRET(IMalloc * pMalloc,
					   LPSTRRET pStrRet, 
					   LPCITEMIDLIST pidl, 
					   LPTSTR pszText, 
					   DWORD dwSize)
{
	if(IsBadReadPtr(pStrRet, sizeof(UINT)))
		return FALSE;
	
	if(IsBadWritePtr(pszText, dwSize))
		return FALSE;
	
	switch(pStrRet->uType)
	{
	case STRRET_CSTR:
		CString::AnsiToLocal(pszText, pStrRet->cStr, dwSize);
		break;
		
	case STRRET_OFFSET:
		lstrcpyn(pszText, (LPTSTR)(((LPBYTE)pidl) + pStrRet->uOffset), dwSize);
		break;
		
	case STRRET_WSTR:
		{
			CString::WideCharToLocal(pszText, pStrRet->pOleStr, dwSize);
			
			if(!pMalloc)
			{
				SHGetMalloc(&pMalloc);
			}
			else
			{
				pMalloc->AddRef();
			}
			if(pMalloc)
			{
				pMalloc->Free(pStrRet->pOleStr);
				pMalloc->Release();
			}
		}
		break;
		
	default:
		return FALSE;
	}
	
	return TRUE;
}

HGLOBAL CDragAndDrop::CreateHDrop(IShellFolder *psfParent, LPITEMIDLIST* aPidls, UINT uItemCount)
{
	HGLOBAL     hGlobal;
	LPDROPFILES pDropFiles;
	UINT        i;
	LPTSTR      pszTemp;
	
	// allocate space for DROPFILE structure plus the number of file and one extra byte for final NULL terminator
	hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (DWORD)(sizeof(DROPFILES) + (MAX_PATH * sizeof(TCHAR) * uItemCount) + sizeof(TCHAR)));
	if(!hGlobal)
	{
		return NULL;
	}
	
	pDropFiles = (LPDROPFILES)GlobalLock(hGlobal);
	
	// set the offset where the starting point of the file start
	pDropFiles->pFiles = sizeof(DROPFILES);
	
	// file contains wide characters?
#ifdef UNICODE
	pDropFiles->fWide = TRUE;
#else
	pDropFiles->fWide = FALSE;
#endif
	
	for(i = 0, pszTemp = (LPTSTR)((LPBYTE)pDropFiles + sizeof(DROPFILES)); i < uItemCount; i++)
	{
		STRRET   str;
		
		//Get the file name
		if(SUCCEEDED(psfParent->GetDisplayNameOf(aPidls[i], SHGDN_FORPARSING, &str)))
		{
			GetTextFromSTRRET(NULL, &str, aPidls[i], pszTemp, MAX_PATH);
			
			pszTemp += lstrlen(pszTemp) + 1;
		}
	}
	
	// final null terminator as per CF_HDROP Format specs
	*pszTemp = 0;
	
	GlobalUnlock(hGlobal);
	
	return hGlobal;
}

HGLOBAL CDragAndDrop::CreatePrivateClipboardData( LPITEMIDLIST pidlParent, 
								   LPITEMIDLIST *aPidls, 
								   UINT uItemCount,
								   BOOL fCut)
{
	HGLOBAL        hGlobal = NULL;
	LPPRIVCLIPDATA pData;
	UINT           iCurPos;
	UINT           cbPidl;
	UINT           i;
	CPidlManager   *pPidlMgr;
	
	pPidlMgr = new CPidlManager();
	
	if(!pPidlMgr)
		return NULL;
	
	//get the size of the parent folder's PIDL
	cbPidl = pPidlMgr->GetSize(pidlParent);
	
	//get the total size of all of the PIDLs
	for(i = 0; i < uItemCount; i++)
	{
		cbPidl += pPidlMgr->GetSize(aPidls[i]);
	}
	
	/*
	Find the end of the PRIVCLIPDATA structure. This is the size of the 
	PRIVCLIPDATA structure itself (which includes one element of aoffset) plus the 
	additional number of elements in aoffset.
	*/
	iCurPos = sizeof(PRIVCLIPDATA) + (uItemCount * sizeof(UINT));
	
	/*
	Allocate the memory for the PRIVCLIPDATA structure and it's variable length members.
	*/
	hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (DWORD)
		(iCurPos +        // size of the PRIVCLIPDATA structure and the additional aoffset elements
		(cbPidl + 1)));   // size of the pidls
	
	if (NULL == hGlobal)
		return (hGlobal);
	
	pData = (LPPRIVCLIPDATA)GlobalLock(hGlobal);
	
	if(pData)
	{
		pData->fCut = fCut;
		pData->cidl = uItemCount;
		pData->aoffset[0] = iCurPos;
		
		//add the PIDL for the parent folder
		cbPidl = pPidlMgr->GetSize(pidlParent);
		CopyMemory((LPBYTE)(pData) + iCurPos, (LPBYTE)pidlParent, cbPidl);
		iCurPos += cbPidl;
		
		for(i = 0; i < uItemCount; i++)
		{
			//get the size of the PIDL
			cbPidl = pPidlMgr->GetSize(aPidls[i]);
			
			//fill out the members of the PRIVCLIPDATA structure.
			pData->aoffset[i + 1] = iCurPos;
			
			//copy the contents of the PIDL
			CopyMemory((LPBYTE)(pData) + iCurPos, (LPBYTE)aPidls[i], cbPidl);
			
			//set up the position of the next PIDL
			iCurPos += cbPidl;
		}
		
		GlobalUnlock(hGlobal);
	}
	
	delete pPidlMgr;
	
	return (hGlobal);
}

HGLOBAL CDragAndDrop::CreateShellIDList( LPITEMIDLIST pidlParent, 
						  LPITEMIDLIST *aPidls, 
						  UINT uItemCount)
{
	HGLOBAL        hGlobal = NULL;
	LPIDA          pData;
	UINT           iCurPos;
	UINT           cbPidl;
	UINT           i;
	CPidlManager   *pPidlMgr;
	
	pPidlMgr = new CPidlManager();

	if(!pPidlMgr)
		return NULL;
	
	//get the size of the parent folder's PIDL
	cbPidl = pPidlMgr->GetSize(pidlParent);
	
	//get the total size of all of the PIDLs
	for(i = 0; i < uItemCount; i++)
	{
		cbPidl += pPidlMgr->GetSize(aPidls[i]);
	}
	
	/*
	Find the end of the CIDA structure. This is the size of the 
	CIDA structure itself (which includes one element of aoffset) plus the 
	additional number of elements in aoffset.
	*/
	iCurPos = sizeof(CIDA) + (uItemCount * sizeof(UINT));
	
	/*
	Allocate the memory for the CIDA structure and it's variable length members.
	*/
	hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (DWORD)
		(iCurPos +        // size of the CIDA structure and the additional aoffset elements
		(cbPidl + 1)));   // size of the pidls
	
	if (NULL == hGlobal)
		return (hGlobal);
	
	pData = (LPIDA)GlobalLock(hGlobal);
	
	if(pData)
	{
		pData->cidl = uItemCount;
		pData->aoffset[0] = iCurPos;
		
		//add the PIDL for the parent folder
		cbPidl = pPidlMgr->GetSize(pidlParent);
		CopyMemory((LPBYTE)(pData) + iCurPos, (LPBYTE)pidlParent, cbPidl);
		iCurPos += cbPidl;
		
		for(i = 0; i < uItemCount; i++)
		{
			//get the size of the PIDL
			cbPidl = pPidlMgr->GetSize(aPidls[i]);
			
			//fill out the members of the CIDA structure.
			pData->aoffset[i + 1] = iCurPos;
			
			//copy the contents of the PIDL
			CopyMemory((LPBYTE)(pData) + iCurPos, (LPBYTE)aPidls[i], cbPidl);
			
			//set up the position of the next PIDL
			iCurPos += cbPidl;
		}
		
		GlobalUnlock(hGlobal);
	}
	
	delete pPidlMgr;
	
	return (hGlobal);
}
