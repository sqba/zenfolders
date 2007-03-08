#ifndef SHELLFOLDER_H
#define SHELLFOLDER_H

#include <windows.h>
#include <shlobj.h>
#include "pidlmgr.h"
//#include "foldprop.h"
#include "util/dragdrop.h"
#include "cfgxml.h"

class CFolderPropertiesDlg;

class CShellFolder : public IShellFolder, public IPersistFolder
{
	friend class CShellView;
	friend class CContextMenu;
	
public:
	CShellFolder();
	CShellFolder(CShellFolder*, LPCITEMIDLIST);
	~CShellFolder();
	
public:
	//IUnknown functions
	STDMETHOD (QueryInterface) (REFIID riid, LPVOID * ppvObj);
	STDMETHOD_ (ULONG, AddRef) (void);
	STDMETHOD_ (ULONG, Release) (void);
	
	//IPersist functions
	STDMETHODIMP GetClassID(LPCLSID);
	
	//IPersistFolder functions
	STDMETHODIMP Initialize(LPCITEMIDLIST);
	
	//IShellFolder functions
	STDMETHOD (ParseDisplayName) (HWND, LPBC, LPOLESTR, LPDWORD, LPITEMIDLIST*, LPDWORD);
	STDMETHOD (EnumObjects) (HWND, DWORD, LPENUMIDLIST*);
	STDMETHOD (BindToObject) (LPCITEMIDLIST, LPBC, REFIID, LPVOID*);
	STDMETHOD (BindToStorage) (LPCITEMIDLIST, LPBC, REFIID, LPVOID*);
	STDMETHOD (CompareIDs) (LPARAM, LPCITEMIDLIST, LPCITEMIDLIST);
	STDMETHOD (CreateViewObject) (HWND, REFIID, LPVOID* );
	STDMETHOD (GetAttributesOf) (UINT, LPCITEMIDLIST*, LPDWORD);
	STDMETHOD (GetUIObjectOf) (HWND, UINT, LPCITEMIDLIST*, REFIID, LPUINT, LPVOID*);
	STDMETHOD (GetDisplayNameOf) (LPCITEMIDLIST, DWORD, LPSTRRET);
	STDMETHOD (SetNameOf) (HWND, LPCITEMIDLIST, LPCOLESTR, DWORD, LPITEMIDLIST*);
	
public:
	BOOL GetFolderPath(LPTSTR, DWORD);
	BOOL RemoveFolder(LPCITEMIDLIST, BOOL bVerify = TRUE);
	void RemoveFolders(LPITEMIDLIST*);
	void ShowProperties(LPCITEMIDLIST);
	void RemoveFiletype(LPCTSTR);
	void ShowOnlyExtension(LPCTSTR);
	void AddExtensionSubfolder(LPCTSTR);
	LPENUMIDLIST CreateList(DWORD, HRESULT*);
	LPITEMIDLIST CreateSubfolder(LPCITEMIDLIST, LPCTSTR);
	void ClearFolderSearch(LPCITEMIDLIST);
	LPITEMIDLIST CreateNewFolder(LPCITEMIDLIST);
	void OpenFolder(HWND, LPCITEMIDLIST, BOOL);
	void Execute(LPCITEMIDLIST);
	void OpenContainingFolder(LPCITEMIDLIST);

	LPITEMIDLIST CreateFQPidl(LPCITEMIDLIST);

	UINT GetFileCount()			{ return m_iFileCount; }
	UINT GetFolderCount()		{ return m_iFolderCount; }

	LPITEMIDLIST GetPidlRel()	{ return m_pidlRel.GetRelative(); }
	LPITEMIDLIST GetPidlFull()	{ return m_pidlRel.GetFull(); }
	
	STDMETHOD (MoveCopyItems)(CShellFolder*, LPITEMIDLIST*, UINT, BOOL);

	BOOL IsRoot();

private:
	void Initialize();
	void DisplayVersion();

private:
	DWORD					m_ObjRefCount;
	CShellFolder			*m_pSFParent;
	CPidl					m_pidlRel;
	CPidl					m_pidlFQ;
	LPMALLOC				m_pMalloc;
//	CFolderPropertiesDlg	*m_pDlg;
	UINT					m_iFileCount;
	UINT					m_iFolderCount;
};


class CFolderPropertiesDlg
{
public:
	CFolderPropertiesDlg(CShellFolder*, LPCITEMIDLIST);
	virtual ~CFolderPropertiesDlg();

	void Show();

	BOOL OnInit(HWND);
	BOOL OnApply(HWND);
	BOOL OnOk(HWND);
	BOOL OnCancel(HWND);
	BOOL OnClose(HWND);

private:
//	void CenterDialog(HWND);
	BOOL ApplyChanges(HWND);

	void InitMaxResults(HWND);
	void InitQueryString(HWND);
	void InitFolderName(HWND);
	void InitRanking(HWND);
	void InitCategory(HWND);

	BOOL SetMaxResults(HWND, LPPIDLDATA);
	BOOL SetQueryString(HWND, LPPIDLDATA);
	BOOL SetFolderName(HWND, LPPIDLDATA);
	BOOL SetRanking(HWND, LPPIDLDATA);
	BOOL SetCategory(HWND, LPPIDLDATA);

	void SetCaption(HWND, LPPIDLDATA);

private:
	HWND			m_hDlg;
//	HINSTANCE		m_hInst;
	CPidl			m_pidl;
	CShellFolder	*m_pParent;
};

BOOL CALLBACK PropertiesDlgProc(HWND, UINT, WPARAM, LPARAM);

#endif   //SHELLFOLDER_H
