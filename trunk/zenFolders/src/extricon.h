#ifndef EXTRACTICON_H
#define EXTRACTICON_H

#include <windows.h>
#include <shlobj.h>
#include "pidlmgr.h"
#include "shlfldr.h"

class CExtractIcon : public IExtractIcon
{
public:
	CExtractIcon(CShellFolder*, LPCITEMIDLIST);
	~CExtractIcon();
	
public:
	//IUnknown methods
	STDMETHOD (QueryInterface) (REFIID, LPVOID*);
	STDMETHOD_ (ULONG, AddRef) (void);
	STDMETHOD_ (ULONG, Release) (void);
	
	//IExtractIcon methods
	STDMETHOD (Extract) (LPCTSTR, UINT, HICON*, HICON*, UINT);
	STDMETHOD (GetIconLocation) (UINT, LPTSTR, UINT, LPINT, LPUINT);

private:
	DWORD			m_ObjRefCount;
	LPITEMIDLIST	m_pidl;
	CShellFolder	*m_pSFParent;
};

#endif   //EXTRACTICON_H
