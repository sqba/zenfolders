
#include <crtdbg.h>
#include "droptrgt.h"
#include "zenfolders.h"


extern LPPIDLMGR	g_pPidlMgr;


#define CFSTR_ZENFOLDERSDATA TEXT("zenFoldersDataFormat")


CDropTarget::CDropTarget(CShellFolder *psfParent)
{
	gAddRef(CDROPTARGET);

	m_psfParent = psfParent;
	if(m_psfParent)
		m_psfParent->AddRef();
	else
	{
		delete this;
		return;
	}
	
	SHGetMalloc(&m_pMalloc);
	if(!m_pMalloc)
	{
		delete this;
		return;
	}
	
	m_ObjRefCount = 1;
	
	m_fAcceptFmt = FALSE;

	m_cfPrivateData = RegisterClipboardFormat(CFSTR_ZENFOLDERSDATA);
}

CDropTarget::~CDropTarget()
{
	if(m_psfParent)
		m_psfParent->Release();
	
	if(m_pMalloc)
		m_pMalloc->Release();

	gRelease(CDROPTARGET);
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

STDMETHODIMP CDropTarget::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;
	
	//IUnknown
	if(IsEqualIID(riid, IID_IUnknown))
	{
		*ppReturn = this;
	}
	
	//IDropTarget
	else if(IsEqualIID(riid, IID_IDropTarget))
	{
		*ppReturn = (IDropTarget*)this;
	}
	
	if(*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}

STDMETHODIMP_(DWORD) CDropTarget::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CDropTarget::Release()
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
// IDropTarget Implementation
//

STDMETHODIMP CDropTarget::DragEnter(LPDATAOBJECT pDataObj,
									DWORD dwKeyState,
									POINTL pt,
									LPDWORD pdwEffect)
{  
	FORMATETC   fmtetc;
	
	fmtetc.cfFormat   = m_cfPrivateData;
	fmtetc.ptd        = NULL;
	fmtetc.dwAspect   = DVASPECT_CONTENT;
	fmtetc.lindex     = -1;
	fmtetc.tymed      = TYMED_HGLOBAL;
	
	//does the drag source provide our private data type?
	if(!(m_fAcceptFmt = (S_OK == pDataObj->QueryGetData(&fmtetc))))
	{
		//try CF_HDROP
		fmtetc.cfFormat   = CF_HDROP;
		m_fAcceptFmt = (S_OK == pDataObj->QueryGetData(&fmtetc));
	}
	
	
	QueryDrop(dwKeyState, pdwEffect);
	
	return S_OK;
}

STDMETHODIMP CDropTarget::DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	QueryDrop(dwKeyState, pdwEffect);
	
	return S_OK;
}

STDMETHODIMP CDropTarget::DragLeave()
{
	m_fAcceptFmt = FALSE;
	
	return S_OK;
}

STDMETHODIMP CDropTarget::Drop(LPDATAOBJECT pDataObj,
							   DWORD dwKeyState,
							   POINTL pt,
							   LPDWORD pdwEffect)
{   
	HRESULT  hr = E_FAIL;
	
	if(QueryDrop(dwKeyState, pdwEffect))
	{      
		FORMATETC   fmtetc;
		STGMEDIUM   medium;
		
		fmtetc.cfFormat   = m_cfPrivateData;
		fmtetc.ptd        = NULL;
		fmtetc.dwAspect   = DVASPECT_CONTENT;
		fmtetc.lindex     = -1;
		fmtetc.tymed      = TYMED_HGLOBAL;
		
		//The user has dropped on us. Get the data from the data object.
		hr = pDataObj->GetData(&fmtetc, &medium);
		if(SUCCEEDED(hr))
		{
			DoPrivateDrop(medium.hGlobal, DROPEFFECT_MOVE == *pdwEffect);
			
			//release the STGMEDIUM
			ReleaseStgMedium(&medium);
			
			return S_OK;
		}
		else
		{
			//try CF_HDROP
			fmtetc.cfFormat = CF_HDROP;
			hr = pDataObj->GetData(&fmtetc, &medium);
			if(SUCCEEDED(hr))
			{
				DoHDrop((HDROP)medium.hGlobal, DROPEFFECT_MOVE == *pdwEffect);
				
				//release the STGMEDIUM
				ReleaseStgMedium(&medium);
				
				return S_OK;
			}
		}
	}
	
	*pdwEffect = DROPEFFECT_NONE;
	
	return hr;
}

BOOL CDropTarget::QueryDrop(DWORD dwKeyState, LPDWORD pdwEffect)
{
	DWORD dwOKEffects = *pdwEffect;
	
	*pdwEffect = DROPEFFECT_NONE;
	
	if(m_fAcceptFmt)
	{
		*pdwEffect = GetDropEffectFromKeyState(dwKeyState);
		
		//we don't accept links
		if(DROPEFFECT_LINK == *pdwEffect)
			*pdwEffect = DROPEFFECT_NONE;
		
		//Check if the drag source application allows the drop effect desired by the 
		//user. The drag source specifies this in DoDragDrop. 
		if(*pdwEffect & dwOKEffects)
			return TRUE;
	}
	
	return FALSE;
}

DWORD CDropTarget::GetDropEffectFromKeyState(DWORD dwKeyState)
{
	//move is the default
	DWORD dwDropEffect = DROPEFFECT_MOVE;
	
	if(dwKeyState & MK_CONTROL)
	{
		if(dwKeyState & MK_SHIFT)
		{
			dwDropEffect = DROPEFFECT_LINK;
		}
		else
		{
			dwDropEffect = DROPEFFECT_COPY;
		}
	}
	
	return dwDropEffect;
}

BOOL CDropTarget::DoPrivateDrop(HGLOBAL hMem, BOOL fMove)
{
	_RPTF0(_CRT_WARN, "CDropTarget::DoPrivateDrop()\n");
	BOOL  fSuccess = FALSE;

	if(hMem)
	{
		LPPRIVCLIPDATA pData = (LPPRIVCLIPDATA)GlobalLock(hMem);
		
		if(pData)
		{
			CShellFolder   *psfFrom = NULL;
			IShellFolder   *psfDesktop;
			LPITEMIDLIST   pidl;
			
			pidl = (LPITEMIDLIST)((LPBYTE)(pData) + pData->aoffset[0]);
			//This is a fully qualified PIDL, so use the desktop folder to get the 
			//IShellFolder for this folder.
			SHGetDesktopFolder(&psfDesktop);
			if(psfDesktop)
			{
				//if the source and the target folders are the same, this is a NOOP
				LPITEMIDLIST pidlParentFQ = m_psfParent->CreateFQPidl(NULL);
				
				if(pidlParentFQ)
				{
					if(0 != psfDesktop->CompareIDs(0, pidl, pidlParentFQ))
					{
						psfDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (LPVOID*)&psfFrom);
					}
					
					g_pPidlMgr->Delete(pidlParentFQ);
				}
				
				psfDesktop->Release();
			}
			
			if(psfFrom)
			{
				LPITEMIDLIST   *aPidls;
				
				//allocate an array of PIDLS
				aPidls = AllocPidlTable(pData->cidl);
				
				if(aPidls)
				{
					UINT  i;
					
					//fill in the PIDL array
					for(i = 0; i < pData->cidl; i++)
					{
						aPidls[i] = g_pPidlMgr->Copy((LPITEMIDLIST)((LPBYTE)(pData) + pData->aoffset[i + 1]));
					}
					
					fSuccess = SUCCEEDED(m_psfParent->MoveCopyItems(psfFrom, aPidls, pData->cidl, fMove));
					
					FreePidlTable(aPidls);
				}
				
				psfFrom->Release();
			}
			
			GlobalUnlock(hMem);
		}
	}

	return fSuccess;
}

BOOL CDropTarget::DoHDrop(HDROP hDrop, BOOL fMove)
{
	_RPTF0(_CRT_WARN, "CDropTarget::DoHDrop()\n");

	TCHAR szFileName[MAX_PATH]={0};
	UINT nFiles = ::DragQueryFile(hDrop, (UINT) -1, NULL, 0);
    for( UINT i=0; i<nFiles; i++ ) 
    {
    	// Get dragged filename
    	if( 0 == ::DragQueryFile(hDrop, i, szFileName, MAX_PATH) ) 
			break;

		_RPTF1(_CRT_WARN, " %s\n", szFileName);

		DWORD dwAttrib = ::GetFileAttributes(szFileName);

		// dragged a folder
		bool bFolder = (0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

		if( bFolder )
		{
			m_psfParent->AddFolderLink( szFileName );
		}
		else
		{
			m_psfParent->AddFileLink( szFileName );
		}
	}
//	return SUCCEEDED(m_psfParent->MoveCopyFromHDrop(hDrop, fMove));

	return FALSE;
}

LPITEMIDLIST* CDropTarget::AllocPidlTable(DWORD dwEntries)
{
	LPITEMIDLIST   *aPidls;
	
	dwEntries++;
	
	aPidls = (LPITEMIDLIST*)m_pMalloc->Alloc(dwEntries * sizeof(LPITEMIDLIST));
	
	if(aPidls)
	{
		//set all of the entries to NULL
		ZeroMemory(aPidls, dwEntries * sizeof(LPITEMIDLIST));
	}
	
	return aPidls;
}

void CDropTarget::FreePidlTable(LPITEMIDLIST *aPidls)
{
	if(aPidls && g_pPidlMgr)
	{
		UINT  i;
		for(i = 0; aPidls[i]; i++)
			g_pPidlMgr->Delete(aPidls[i]);
		
		m_pMalloc->Free(aPidls);
	}
}
