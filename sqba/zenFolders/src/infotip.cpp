
#include "infotip.h"

//#if (_WIN32_IE >= 0x0400)

#include "guid.h"
#include "util/string.h"
#include "zenfolders.h"


extern HINSTANCE	g_hInst;
extern LPPIDLMGR	g_pPidlMgr;


CQueryInfo::CQueryInfo(LPCITEMIDLIST pidl)
{
	//get the shell's IMalloc pointer
	//we'll keep this until we get destroyed
	if(FAILED(SHGetMalloc(&m_pMalloc)))
	{
		delete this;
		return;
	}
	
	m_pidl = g_pPidlMgr->Copy(pidl);
	
	m_ObjRefCount = 1;

	gAddRef(CQUERYINFO);
}

CQueryInfo::~CQueryInfo()
{
	if(m_pidl)
	{
		g_pPidlMgr->Delete(m_pidl);
		m_pidl = NULL;
	}
	
	if(m_pMalloc)
		m_pMalloc->Release();

	gRelease(CQUERYINFO);
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

STDMETHODIMP CQueryInfo::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;
	
	//IUnknown
	if(IsEqualIID(riid, IID_IUnknown))
		*ppReturn = this;
	
	//IQueryInfo
	else if(IsEqualIID(riid, IID_IQueryInfo))
		*ppReturn = (IQueryInfo*)this;
	
	if(*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CQueryInfo::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CQueryInfo::Release()
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
// IQueryInfo Implementation
//

/**************************************************************************
IQueryInfo::GetInfoFlags
	Retrieves the information flags for an item.
	This method is not currently used.

Parameters
	pdwFlags [out]	Pointer to a value that receives the flags for the item.
					If no flags are to be returned, this value should be
					set to zero.

Return Value
	Returns S_OK if pdwFlags returns any flag values, or an OLE-defined
	error value otherwise.
**************************************************************************/
STDMETHODIMP CQueryInfo::GetInfoFlags(LPDWORD pdwFlags)
{
	return E_NOTIMPL;
}

/**************************************************************************
IQueryInfo::GetInfoTip
	Retrieves the info tip text for an item.

Parameters
	dwFlags			Flags directing the handling of the item from which
					you're retrieving the info tip text. This value is
					commonly 0 (QITIPF_DEFAULT).
        QITIPF_DEFAULT		No special handling.
        QITIPF_USENAME		Provide the name of the item in ppwszTip
							rather than the info tip text.
        QITIPF_LINKNOTARGET	If the item is a shortcut, retrieve the info
							tip text of the shortcut rather than its target.
        QITIPF_LINKUSETARGET	If the item is a shortcut, retrieve the
								info tip text of the shortcut's target.
        QITIPF_USESLOWTIP	Search the entire namespace for the information.
							This may result in a delayed response time.
        QITIPF_SINGLELINE	Put the info tip on a single line.

	ppwszTip [out]	Address of a Unicode string pointer that receives
					the tip string pointer. Applications that implement
					this method must allocate memory for ppwszTip by
					calling SHGetMalloc. Calling applications must call
					SHGetMalloc to free the memory when it is no longer needed. 

Return Value
	Returns S_OK if the function succeeds. If no info tip text is available,
	ppwszTip is set to NULL. Otherwise, returns an OLE-defined error value.
**************************************************************************/
STDMETHODIMP CQueryInfo::GetInfoTip(DWORD dwFlags, WCHAR **ppwszTip)
{
	TCHAR szTipText[MAX_PATH];
	int   cchOleStr;
	
	*ppwszTip = NULL;
	
	//get the entire text for the item
	CPidlManager::GetPidlPath(m_pidl, szTipText, sizeof(szTipText));
	
	//get the number of characters required
	cchOleStr = lstrlen(szTipText) + 1;
	
	//allocate the wide character string
	*ppwszTip = (LPWSTR)m_pMalloc->Alloc(cchOleStr * sizeof(WCHAR));
	if(!*ppwszTip)
		return E_OUTOFMEMORY;
	
	CString::LocalToWideChar(*ppwszTip, szTipText, cchOleStr);
	
	return S_OK;
}

//#endif   //(_WIN32_IE >= 0x0400)
