#ifndef ENUMIDLIST_H
#define ENUMIDLIST_H

#include <windows.h>
#include <shlobj.h>
#include "pidlmgr.h"
#include "cfgxml.h"

typedef struct _ENUMLIST
{
	struct _ENUMLIST	*pNext;
	LPITEMIDLIST		pidl;
} ENUMLIST, FAR *LPENUMLIST;

class CEnumIDList : public IEnumIDList
{
public:
	CEnumIDList(LPCITEMIDLIST, DWORD, HRESULT*);
	~CEnumIDList();
	
public:
	//IUnknown methods
	STDMETHOD (QueryInterface)(REFIID, LPVOID*);
	STDMETHOD_ (DWORD, AddRef)();
	STDMETHOD_ (DWORD, Release)();
	
	//IEnumIDList
	STDMETHOD (Clone) (LPENUMIDLIST*);
	STDMETHOD (Next) (DWORD, LPITEMIDLIST*, LPDWORD);
	STDMETHOD (Reset) (void);
	STDMETHOD (Skip) (DWORD);

public:
	UINT GetFileCount()		{ return m_iFileCount; }
	UINT GetFolderCount()	{ return m_iFolderCount; }
	
private:
	BOOL AddToEnumList(LPITEMIDLIST);
	BOOL DeleteList(void);
	BOOL EnumerateFolders(MSXML2::IXMLDOMNodePtr);
	BOOL EnumerateFiles(MSXML2::IXMLDOMNodePtr);
	BOOL EnumerateFolderLinks(MSXML2::IXMLDOMNodePtr);
	BOOL EnumerateFileLinks(MSXML2::IXMLDOMNodePtr);
	BOOL EnumerateDirectory(LPCTSTR, DWORD);

private:
	DWORD		m_ObjRefCount;
	LPMALLOC	m_pMalloc;
	LPENUMLIST	m_pFirst;
	LPENUMLIST	m_pLast;
	LPENUMLIST	m_pCurrent;
	UINT		m_iFileCount;
	UINT		m_iFolderCount;
};

#endif   //ENUMIDLIST_H
