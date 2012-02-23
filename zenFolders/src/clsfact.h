#ifndef CLASSFACTORY_H
#define CLASSFACTORY_H

#include <windows.h>

class CClassFactory : public IClassFactory
{
public:
	CClassFactory();
	~CClassFactory();
	
public:
	//IUnknown methods
	STDMETHODIMP QueryInterface(REFIID, LPVOID*);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();
	
	//IClassFactory methods
	STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID*);
	STDMETHODIMP LockServer(BOOL);

private:
	DWORD m_ObjRefCount;
};

#endif   //CLASSFACTORY_H
