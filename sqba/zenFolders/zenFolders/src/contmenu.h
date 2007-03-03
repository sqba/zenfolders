#ifndef CONTMENU_H
#define CONTMENU_H

#include <windows.h>
#include <shlobj.h>
#include "shlfldr.h"

class CContextMenu : public IContextMenu
{
public:
	CContextMenu(CShellFolder*, LPCITEMIDLIST*, UINT);
	~CContextMenu();
	
public:
	//IUnknown methods
	STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();
	
	//IContextMenu methods
	STDMETHODIMP GetCommandString(UINT, UINT, LPUINT, LPSTR, UINT);
	STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO);
	STDMETHODIMP QueryContextMenu(HMENU, UINT, UINT, UINT, UINT);

private:
	BOOL AllocPidlTable(DWORD);
	void FreePidlTable(void);
	BOOL FillPidlTable(LPCITEMIDLIST*, UINT);
	BOOL CanRenameItems(void);
	void AddMenuItem(HMENU, LPTSTR, UINT, UINT=0, BOOL=TRUE, BOOL=FALSE);
	void OnExecute(LPCITEMIDLIST);
	void OnOpenFolder(LPCITEMIDLIST, HWND, bool);
	void OnShowProperties(LPCITEMIDLIST);
	void OnCreateExtensionFolder(LPCITEMIDLIST);
	void OnHideExtension(LPCITEMIDLIST);
	void OnCreateNewFolder(LPCITEMIDLIST);
	void OnShowOnlyExtension(LPCITEMIDLIST);
	void OnOpenContainingFolder(LPCITEMIDLIST);
	void OnClearSearch(LPCITEMIDLIST);
	void OnRemoveFolders(LPITEMIDLIST*);

private:
	DWORD			m_ObjRefCount;
	LPITEMIDLIST	*m_aPidls;
	IMalloc			*m_pMalloc;
	CShellFolder	*m_pSFParent;
	BOOL			m_fAllFiles;
};

#endif// CONTMENU_H
