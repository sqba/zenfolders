
#include <crtdbg.h>

#include "clsfact.h"
#include "shlfldr.h"
#include "zenfolders.h"


CClassFactory::CClassFactory()
{
	m_ObjRefCount = 1;
	gAddRef(CCLASSFACTORY);
}

CClassFactory::~CClassFactory()
{
	gRelease(CCLASSFACTORY);
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;
	
	if(IsEqualIID(riid, IID_IUnknown))
		*ppReturn = this;
	
	else if(IsEqualIID(riid, IID_IClassFactory))
		*ppReturn = (IClassFactory*)this;
	
	if(*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CClassFactory::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CClassFactory::Release()
{
	if(--m_ObjRefCount == 0)
	{
		delete this;
		return 0;
	}
	
	return m_ObjRefCount;
}

STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN pUnknown, 
										   REFIID riid, 
										   LPVOID *ppObject)
{
	*ppObject = NULL;
	
	if(pUnknown != NULL)
		return CLASS_E_NOAGGREGATION;
	
	//add implementation specific code here
	
	CShellFolder *pShellFolder = new CShellFolder();
	if(NULL == pShellFolder)
		return E_OUTOFMEMORY;
	
	//get the QueryInterface return for our return value
	HRESULT hResult = pShellFolder->QueryInterface(riid, ppObject);
	
	//call Release to decement the ref count
	pShellFolder->Release();
	
	//return the result from QueryInterface
	return hResult;
}

STDMETHODIMP CClassFactory::LockServer(BOOL)
{
	return E_NOTIMPL;
}
