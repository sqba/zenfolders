#ifndef SHELLVIEW_H
#define SHELLVIEW_H

#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <shlobj.h>
#include <servprov.h>
#include <docobj.h>
#include "shlfldr.h"
#include "contmenu.h"
#include "listview.h"
#include "browser.h"

class CShellView : public IShellView, public IOleCommandTarget
{
public:
	CShellView(CShellFolder*, LPCITEMIDLIST);
	~CShellView();

public:
	//IUnknown methods
	STDMETHOD (QueryInterface)(REFIID, LPVOID*);
	STDMETHOD_ (DWORD, AddRef)();
	STDMETHOD_ (DWORD, Release)();

	//IOleWindow methods
	STDMETHOD (GetWindow) (HWND*);
	STDMETHOD (ContextSensitiveHelp) (BOOL);

	//IShellView methods
	STDMETHOD (TranslateAccelerator) (LPMSG);
	STDMETHOD (EnableModeless) (BOOL);
	STDMETHOD (UIActivate) (UINT);
	STDMETHOD (Refresh) (void);
	STDMETHOD (CreateViewWindow) (LPSHELLVIEW, LPCFOLDERSETTINGS, LPSHELLBROWSER, LPRECT, HWND*);
	STDMETHOD (DestroyViewWindow) (void);
	STDMETHOD (GetCurrentInfo) (LPFOLDERSETTINGS);
	STDMETHOD (AddPropertySheetPages) (DWORD, LPFNADDPROPSHEETPAGE, LPARAM);
	STDMETHOD (SaveViewState) (void);
	STDMETHOD (SelectItem) (LPCITEMIDLIST, UINT);
	STDMETHOD (GetItemObject) (UINT, REFIID, LPVOID*);

	//IOleCommandTarget methods
	STDMETHOD (QueryStatus) (const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText);
	STDMETHOD (Exec) (const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);

public:
	void AddNewFolder(LPCITEMIDLIST);

private:
	int OnBeginDrag();
	int OnGetDispInfo(LPNMHDR);
	int OnRightClick(void);
	void OnEndTrack(void);
	void OnDeleteItem(LPNMHDR);
	void OnDeactivate(void);
	void OnHeaderClick(LPNMHEADER);
	BOOL OnBeginLabelEdit(NMLVDISPINFO*);
	void OnEndLabelEdit(NMLVDISPINFO*);
	void OnDefaultAction();
	void OnItemChanged(LPNMLISTVIEW);
	LRESULT OnInitMenuPopup(HMENU hMenu);
	LRESULT OnCommand(DWORD, DWORD, HWND);
	LRESULT OnActivate(UINT);
	LRESULT OnSize(WORD, WORD);
	LRESULT OnCreate(void);
	LRESULT OnKillFocus(void);
	LRESULT OnSetFocus(void);
	LRESULT OnSettingChange(LPCTSTR);
	LRESULT OnNotify(WPARAM, LPARAM);

	void OnRemoveFolders();
	void OnCreateNewFolder();
	void OnShowProperties();
	void OnRename();

	void OnToolbarDropdown();
	void OnSetViewStyle(LONG);

	int FindItemPidl(LPCITEMIDLIST);
	int InsertItem(LPCITEMIDLIST);
	BOOL InitList(void);
	BOOL CreateList(void);
	HMENU BuildMenu(void);
	VOID UpdateData(LPCITEMIDLIST);
	void FillList(void);
	void InitMenu(UINT);
	void MergeViewMenu(HMENU);
	void MergeFileMenu(HMENU);
	void UpdateShellSettings(void);
	void DoContextMenu(WORD, WORD, BOOL);
	HRESULT StateChange(ULONG uChange);

	VOID MergeToolbar(VOID);
	LRESULT UpdateToolbar(VOID);
	void SetListViewStyle();

private:
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

private:
	DWORD			m_ObjRefCount;
	UINT			m_uState;
	LPITEMIDLIST	m_pidl;
	FOLDERSETTINGS	m_FolderSettings;
	LPSHELLBROWSER	m_pShellBrowser;
	HWND			m_hwndParent;
	HWND			m_hWnd;
	HMENU			m_hMenu;
	CShellFolder	*m_pSFParent;
	LPMALLOC		m_pMalloc;
	ICommDlgBrowser *m_pCommDlgBrowser;
	CListView		*m_pListView;
	CWebBrowser		*m_pWebBrowser;
	HACCEL			m_hAccels;
	BOOL			m_bInEdit;
	int				m_iLastSelectedItem;
};

#endif   //SHELLVIEW_H
