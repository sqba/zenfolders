// dataobj.cpp: implementation of the CDataObject class.
//
//////////////////////////////////////////////////////////////////////

#include "dataobj.h"
#include "enumfe.h"
#include "zenfolders.h"

#define CFSTR_ZENFOLDERSDATA TEXT("zenFoldersDataFormat")

extern LPPIDLMGR	g_pPidlMgr;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataObject::CDataObject(CShellFolder *psfParent,
						 LPCITEMIDLIST *aPidls,
						 UINT uItemCount)
{
	gAddRef(CDATAOBJECT);

	m_uItemCount = 0;

	m_psfParent = psfParent;
	if(m_psfParent)
		m_psfParent->AddRef();

	m_ObjRefCount = 1;

	m_aPidls = NULL;
	SHGetMalloc(&m_pMalloc);
	if(!m_pMalloc)
	{
		delete this;
		return;
	}
	
	m_uItemCount = uItemCount;
	
	AllocPidlTable(uItemCount);
	if(m_aPidls)
	{
		FillPidlTable(aPidls, uItemCount);
	}
	
	m_cfPrivateData = RegisterClipboardFormat(CFSTR_ZENFOLDERSDATA);
	m_cfShellIDList = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	m_cfInShellDragLoop = RegisterClipboardFormat(CFSTR_INDRAGLOOP);
	
	m_cFormatEtc = 4;
	m_pFormatEtc = new FORMATETC[m_cFormatEtc];
	SETDefFormatEtc(m_pFormatEtc[0], m_cfPrivateData, TYMED_HGLOBAL);
	SETDefFormatEtc(m_pFormatEtc[1], m_cfShellIDList, TYMED_HGLOBAL);
	SETDefFormatEtc(m_pFormatEtc[2], CF_HDROP, TYMED_HGLOBAL);
	SETDefFormatEtc(m_pFormatEtc[3], m_cfInShellDragLoop, TYMED_HGLOBAL);
}

CDataObject::~CDataObject()
{
	if(m_psfParent)
		m_psfParent->Release();
	
	gRelease(CDATAOBJECT);

	//make sure the pidls are freed
	if(m_aPidls && m_pMalloc)
	{
		FreePidlTable();
	}
	
	if(m_pMalloc)
		m_pMalloc->Release();
	
	//delete [] m_pFormatEtc;
	delete m_pFormatEtc;
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

STDMETHODIMP CDataObject::QueryInterface(   REFIID riid, 
                                            LPVOID *ppReturn)
{
	*ppReturn = NULL;
	
	if(IsEqualIID(riid, IID_IUnknown))
	{
		*ppReturn = (LPUNKNOWN)(LPCONTEXTMENU)this;
	}
	else if(IsEqualIID(riid, IID_IDataObject))
	{
		*ppReturn = (LPDATAOBJECT)this;
	}   
	
	if(*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CDataObject::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CDataObject::Release()
{
	if(--m_ObjRefCount == 0)
		delete this;
	
	return m_ObjRefCount;
}

///////////////////////////////////////////////////////////////////////////
//
// IDataObject Implementation
//

STDMETHODIMP CDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
	if(pFE->cfFormat == CF_HDROP)
	{
		LPITEMIDLIST pidlParentFQ;
		
		pidlParentFQ = m_psfParent->CreateFQPidl(NULL);
		
		if(pidlParentFQ)
		{
			pStgMedium->pUnkForRelease = NULL;
			pStgMedium->hGlobal = CDragAndDrop::CreateHDrop(m_psfParent, m_aPidls, m_uItemCount);
			
			g_pPidlMgr->Delete(pidlParentFQ);
			
			if(pStgMedium->hGlobal)
			{
				pStgMedium->tymed = TYMED_HGLOBAL;
				return S_OK;
			}
		}
		
		return E_OUTOFMEMORY;
	}
	
	else if(pFE->cfFormat == m_cfPrivateData)
	{
		LPITEMIDLIST pidlParentFQ;
		
		pidlParentFQ = m_psfParent->CreateFQPidl(NULL);
		
		if(pidlParentFQ)
		{
			pStgMedium->pUnkForRelease = NULL;
			pStgMedium->hGlobal = CDragAndDrop::CreatePrivateClipboardData(pidlParentFQ, m_aPidls, m_uItemCount, FALSE);
			
			g_pPidlMgr->Delete(pidlParentFQ);
			
			if(pStgMedium->hGlobal)
			{
				pStgMedium->tymed = TYMED_HGLOBAL;
				return S_OK;
			}
		}
		
		return E_OUTOFMEMORY;
	}
	
	else if(pFE->cfFormat == m_cfShellIDList)
	{
		LPITEMIDLIST pidlParentFQ;
		
		pidlParentFQ = m_psfParent->CreateFQPidl(NULL);
		
		if(pidlParentFQ)
		{
			pStgMedium->pUnkForRelease = NULL;
			pStgMedium->hGlobal = CDragAndDrop::CreateShellIDList(pidlParentFQ, m_aPidls, m_uItemCount);
			
			g_pPidlMgr->Delete(pidlParentFQ);
			
			if(pStgMedium->hGlobal)
			{
				pStgMedium->tymed = TYMED_HGLOBAL;
				return S_OK;
			}
		}
		
		return E_OUTOFMEMORY;
	}
	
	else if(pFE->cfFormat == m_cfInShellDragLoop)
	{
		pStgMedium->pUnkForRelease = NULL;
		pStgMedium->hGlobal = GlobalAlloc(GHND | GMEM_SHARE, sizeof(DWORD));
		
		if(pStgMedium->hGlobal)
		{
			LPDWORD  pdw = (LPDWORD)GlobalLock(pStgMedium->hGlobal);
			if(pdw)
			{
				*pdw = TRUE;
				GlobalUnlock(pStgMedium->hGlobal);
			}
			
			pStgMedium->tymed = TYMED_HGLOBAL;
			return S_OK;
		}
		
		return E_OUTOFMEMORY;
	}

	return E_INVALIDARG;
}

/**************************************************************************

   CDataObject::GetDataHere()

**************************************************************************/

STDMETHODIMP CDataObject::GetDataHere (LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
	return E_NOTIMPL;
}

/**************************************************************************

   CDataObject::QueryGetData()

**************************************************************************/

STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC pFE)
{
	BOOL fReturn = FALSE;
	
	// Check the aspects we support. Implementations of this object will only
	// support DVASPECT_CONTENT.
	if(!(DVASPECT_CONTENT & pFE->dwAspect))
		return DV_E_DVASPECT;
	
	if(pFE->cfFormat == CF_HDROP)
	{
		//
		// Now check for an appropriate TYMED.
		//
		for(UINT i = 0; i < m_cFormatEtc; i++)
		{
			fReturn |= m_pFormatEtc[i].tymed & pFE->tymed;
		}
	}
	else if(pFE->cfFormat == m_cfPrivateData)
	{
		//
		// Now check for an appropriate TYMED.
		//
		for(UINT i = 0; i < m_cFormatEtc; i++)
		{
			fReturn |= m_pFormatEtc[i].tymed & pFE->tymed;
		}
	}
	
	else if(pFE->cfFormat == m_cfShellIDList)
	{
		//
		// Now check for an appropriate TYMED.
		//
		for(UINT i = 0; i < m_cFormatEtc; i++)
		{
			fReturn |= m_pFormatEtc[i].tymed & pFE->tymed;
		}
	}
	
	else if(pFE->cfFormat == m_cfInShellDragLoop)
	{
		//
		// Now check for an appropriate TYMED.
		//
		for(UINT i = 0; i < m_cFormatEtc; i++)
		{
			fReturn |= m_pFormatEtc[i].tymed & pFE->tymed;
		}
	}
	
	return (fReturn ? S_OK : DV_E_TYMED);
}

/**************************************************************************

   CDataObject::GetCanonicalFormatEtc()

**************************************************************************/

STDMETHODIMP CDataObject::GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut)
{
	if(NULL == pFEOut)
		return E_INVALIDARG;
	
	pFEOut->ptd = NULL;
	
	return DATA_S_SAMEFORMATETC;
}

/**************************************************************************

   CDataObject::EnumFormatEtc()

**************************************************************************/

STDMETHODIMP CDataObject::EnumFormatEtc(  DWORD dwDirection, 
                                          IEnumFORMATETC** ppEFE)
{
	HRESULT  hr = E_NOTIMPL;
	*ppEFE = NULL;
	
	if(DATADIR_GET == dwDirection)
	{
		CEnumFormatEtc *pefeTemp = new CEnumFormatEtc(m_pFormatEtc, m_cFormatEtc);
		if(pefeTemp)
		{
			hr = pefeTemp->QueryInterface(IID_IEnumFORMATETC, (LPVOID*)ppEFE);
			pefeTemp->Release();
		}
	}
	
	return hr;
}

/**************************************************************************

   CDataObject::SetData()

**************************************************************************/

STDMETHODIMP CDataObject::SetData(  LPFORMATETC pFE, 
                                    LPSTGMEDIUM pStgMedium, 
                                    BOOL fRelease)
{
	return E_NOTIMPL;
}

/**************************************************************************

   CDataObject::DAdvise()

**************************************************************************/

STDMETHODIMP CDataObject::DAdvise(  LPFORMATETC pFE, 
                                    DWORD advf, 
                                    IAdviseSink *ppAdviseSink, 
                                    LPDWORD pdwConnection)
{
	return E_NOTIMPL;
}

/**************************************************************************

   CDataObject::DUnadvise()

**************************************************************************/

STDMETHODIMP CDataObject::DUnadvise(DWORD dwConnection)
{
	return E_NOTIMPL;
}

/**************************************************************************

   CDataObject::EnumDAdvise()

**************************************************************************/

STDMETHODIMP CDataObject::EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
{
	return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////
//
// private and utility methods
//

/**************************************************************************

   CDataObject::AllocPidlTable()

**************************************************************************/

BOOL CDataObject::AllocPidlTable(DWORD dwEntries)
{
	m_aPidls = (LPITEMIDLIST*)m_pMalloc->Alloc(dwEntries * sizeof(LPITEMIDLIST));
	
	if(m_aPidls)
	{
		//set all of the entries to NULL
		ZeroMemory(m_aPidls, dwEntries * sizeof(LPITEMIDLIST));
	}
	
	return (m_aPidls != NULL);
}

/**************************************************************************

   CDataObject::FreePidlTable()

**************************************************************************/

void CDataObject::FreePidlTable()
{
	if(m_aPidls && g_pPidlMgr)
	{
		UINT  i;
		for(i = 0; i < m_uItemCount; i++)
			g_pPidlMgr->Delete(m_aPidls[i]);
		
		m_pMalloc->Free(m_aPidls);
		
		m_aPidls = NULL;
	}
}

/**************************************************************************

   CDataObject::FillPidlTable()

**************************************************************************/

BOOL CDataObject::FillPidlTable(LPCITEMIDLIST *aPidls, UINT uItemCount)
{
	if(m_aPidls)
	{
		if(g_pPidlMgr)
		{
			UINT  i;
			for(i = 0; i < uItemCount; i++)
			{
				m_aPidls[i] = g_pPidlMgr->Copy(aPidls[i]);
			}
			return TRUE;
		}
	}
	
	return FALSE;
}
