#include "enumfe.h"
#include "zenfolders.h"


CEnumFormatEtc::CEnumFormatEtc(FORMATETC *pfe, UINT uFECount)
{
	gAddRef(CENUMFORMATETC);
	
	m_ObjRefCount = 1;
	
	m_cFormatEtc = uFECount;
	m_pFormatEtc = new FORMATETC[m_cFormatEtc];
	CopyMemory(m_pFormatEtc, pfe, sizeof(FORMATETC) * m_cFormatEtc);
	
	m_uCurrent = 0;
}

CEnumFormatEtc::~CEnumFormatEtc()
{
	gRelease(CENUMFORMATETC);
	delete m_pFormatEtc;
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

STDMETHODIMP CEnumFormatEtc::QueryInterface( REFIID riid, 
                                             LPVOID *ppReturn)
{
	*ppReturn = NULL;
	
	if(IsEqualIID(riid, IID_IUnknown))
	{
		*ppReturn = (LPUNKNOWN)this;
	}
	else if(IsEqualIID(riid, IID_IEnumFORMATETC))
	{
		*ppReturn = (LPENUMFORMATETC)this;
	}   
	
	if(*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CEnumFormatEtc::AddRef()
{
	return ++m_ObjRefCount;
}


STDMETHODIMP_(DWORD) CEnumFormatEtc::Release()
{
	if(--m_ObjRefCount == 0)
		delete this;
	
	return m_ObjRefCount;
}

///////////////////////////////////////////////////////////////////////////
//
// IEnumFORMATETC Implementation
//

STDMETHODIMP CEnumFormatEtc::Next(ULONG uRequested, LPFORMATETC pFormatEtc, ULONG* pulFetched)
{
	if(NULL == m_pFormatEtc)
		return S_FALSE;
	
	if(NULL != pulFetched)
		*pulFetched = 0L;
	
	if(NULL == pFormatEtc)
		return E_INVALIDARG;
	
	ULONG uFetched;
	for(uFetched = 0; m_uCurrent < m_cFormatEtc && uRequested > uFetched; uFetched++)
	{
		*pFormatEtc++ = m_pFormatEtc[m_uCurrent++];
	}
	
	if(NULL != pulFetched)
		*pulFetched = uFetched;
	
	return ((uFetched == uRequested) ? S_OK : S_FALSE);
}

STDMETHODIMP CEnumFormatEtc::Skip(ULONG cSkip)
{
	if((m_uCurrent + cSkip) >= m_cFormatEtc)
		return S_FALSE;
	
	m_uCurrent += cSkip;
	
	return S_OK;
}

STDMETHODIMP CEnumFormatEtc::Reset(void)
{
	m_uCurrent = 0;
	return S_OK;
}

STDMETHODIMP CEnumFormatEtc::Clone(LPENUMFORMATETC* ppEnum)
{
	CEnumFormatEtc* pNew;
	
	*ppEnum = NULL;
	
	// Create the clone.
	pNew = new CEnumFormatEtc(m_pFormatEtc, m_cFormatEtc);
	if (NULL == pNew)
		return E_OUTOFMEMORY;
	
	pNew->m_uCurrent = m_uCurrent;
	
	*ppEnum = pNew;
	
	return S_OK;
}
