#ifndef INFOTIP_H
#define INFOTIP_H

#include <windows.h>
#include <shlobj.h>
#include "pidlmgr.h"

class CQueryInfo : public IQueryInfo
{
public:
	CQueryInfo(LPCITEMIDLIST);
	~CQueryInfo();
	
public:
	//IUnknown methods
	STDMETHOD (QueryInterface) (REFIID, LPVOID*);
	STDMETHOD_ (ULONG, AddRef) (void);
	STDMETHOD_ (ULONG, Release) (void);
	
	//IQueryInfo methods
	STDMETHOD(GetInfoFlags)(LPDWORD);
	STDMETHOD(GetInfoTip)(DWORD, WCHAR**);
	
private:
	DWORD			m_ObjRefCount;
	LPITEMIDLIST	m_pidl;
	LPMALLOC		m_pMalloc;
};

#endif   //INFOTIP_H
