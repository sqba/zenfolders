#ifndef SHELLFOLDER_H
#define SHELLFOLDER_H

#include <windows.h>
#include <shlobj.h>
#include "pidlmgr.h"
//#include "foldprop.h"
#include "util/dragdrop.h"
#include "cfgxml.h"
#include "icons.h"

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
	void ShowProperties(LPCITEMIDLIST);
	void RemoveFiletype(LPCTSTR);
	void ShowOnlyExtension(LPCTSTR);
	void AddExtensionSubfolder(LPCTSTR);
	void ClearFolderSearch(LPCITEMIDLIST);
	LPITEMIDLIST CreateNewFolder(LPCITEMIDLIST);
	void OpenFolder(HWND, LPCITEMIDLIST, BOOL);
	void OpenContainingFolder(LPCITEMIDLIST);
	void Delete(LPITEMIDLIST*);
	LPITEMIDLIST GetPidlRel()			{ return m_pidlRel.GetRelative(); }

	void Execute(LPCITEMIDLIST);

	BOOL Rename(LPCITEMIDLIST, LPCTSTR);

	BOOL DeleteFolder(LPCITEMIDLIST, BOOL bVerify = TRUE);
	BOOL DeleteFolderLink(LPCITEMIDLIST, BOOL bVerify = TRUE);

	LPITEMIDLIST CreateFQPidl(LPCITEMIDLIST);

	UINT GetFileCount()					{ return m_iFileCount; }
	UINT GetSubFolderCount()			{ return m_iFolderCount; }
	
	STDMETHOD (MoveCopyItems)(CShellFolder*, LPITEMIDLIST*, UINT, BOOL);

	BOOL IsRoot();

	void SaveListViewStyle(LONG);
	LONG GetListViewStyle();

	HICON GetIconLarge(UINT index)		{ return m_pIcons->GetIconLarge(index); }
	HICON GetIconSmall(UINT index)		{ return m_pIcons->GetIconSmall(index); }
	HIMAGELIST GetListSmall()			{ return m_pIcons->GetListSmall(); }
	HIMAGELIST GetListLarge()			{ return m_pIcons->GetListLarge(); }
	int GetIconIndex(LPCTSTR pszPath)	{ return m_pIcons->GetIconIndex(pszPath); }

	void AddFolderLink(LPCTSTR);
	void AddFileLink(LPCTSTR);

private:
	bool Init();
	void DisplayVersion();
	void DisplayFolderProperties(LPCITEMIDLIST);
	void DisplayShellProperties(LPCITEMIDLIST);
	bool ConfirmDelete(LPITEMIDLIST*);
	LPENUMIDLIST CreateList(DWORD, HRESULT*);
	LPITEMIDLIST CreateSubfolder(LPCITEMIDLIST, LPCTSTR);
	BOOL RenameSubFolder(LPCITEMIDLIST, LPCTSTR);
	BOOL RenameFile(LPCITEMIDLIST, LPCTSTR);
	BOOL DeletePidl(LPCITEMIDLIST, BOOL);
	BOOL DeleteFile(LPCITEMIDLIST, BOOL);

private:
	DWORD			m_ObjRefCount;
	CShellFolder	*m_pSFParent;
	CPidl			m_pidlRel;
	CPidl			m_pidlFQ;
	LPMALLOC		m_pMalloc;
	UINT			m_iFileCount;
	UINT			m_iFolderCount;
	CIconList		*m_pIcons;
};


class CFolderPropertiesDlg
{
public:
	CFolderPropertiesDlg(CShellFolder*, LPCITEMIDLIST);
	CFolderPropertiesDlg(CShellFolder*);
	virtual ~CFolderPropertiesDlg();

	void Show();
	bool Equals(CFolderPropertiesDlg *pDlg);
	void BringToFront();

	bool OnInit(HWND);
	bool OnApply(HWND);
	bool OnOk(HWND);
	bool OnCancel(HWND);
	bool OnClose(HWND);
	bool OnDestroy(HWND);

private:
	static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

private:
	bool ApplyChanges(HWND);

	void InitMaxResults(HWND);
	void InitQueryString(HWND);
	void InitFolderName(HWND);
	void InitRanking(HWND);
	void InitCategory(HWND);

	bool SetMaxResults(HWND, LPPIDLDATA);
	bool SetQueryString(HWND, LPPIDLDATA);
	bool SetFolderName(HWND, LPPIDLDATA);
	bool SetRanking(HWND, LPPIDLDATA);
	bool SetCategory(HWND, LPPIDLDATA);

	void SetCaption(HWND, LPPIDLDATA);

	void DisplayPath(HWND);
	void DisplayVersion(HWND);

private:
	HWND			m_hDlg;
	CPidl			m_pidl;
	CShellFolder	*m_pParent;
};


#endif   //SHELLFOLDER_H
