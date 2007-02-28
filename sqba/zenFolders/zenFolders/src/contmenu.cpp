
#include <crtdbg.h>

#include "commands.h"
#include "contmenu.h"
#include "cfgxml.h"
#include "viewlist.h"
#include "util/string.h"
#include "zenfolders.h"
#include "resource.h"


#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


extern LPCONFIGXML	g_pConfigXML;
extern LPVIEWSLIST	g_pViewList;
extern LPPIDLMGR	g_pPidlMgr;
extern HINSTANCE	g_hInst;


CContextMenu::CContextMenu(CShellFolder *pSFParent,
						   LPCITEMIDLIST *aPidls,
						   UINT uItemCount)
{
	m_pSFParent = pSFParent;
	if(m_pSFParent)
		m_pSFParent->AddRef();
	
	m_ObjRefCount = 1;
	gAddRef(CCONTEXTMENU);
	
	m_aPidls = NULL;

	SHGetMalloc(&m_pMalloc);
	if(!m_pMalloc)
	{
		delete this;
		return;
	}
	
	AllocPidlTable(uItemCount);
	if(m_aPidls)
	{
		FillPidlTable(aPidls, uItemCount);
	}
	
	m_fAllValues = 1;
	UINT  u;
	for(u = 0; u < uItemCount; u++)
	{
		m_fAllValues &= (CPidlManager::IsFile(aPidls[u]) ? 1 : 0);
	}

//	_RPTF1(_CRT_WARN, "CContextMenu(%d)\n", g_DllRefCount);
}

CContextMenu::~CContextMenu()
{
	if(m_pSFParent)
		m_pSFParent->Release();
	
	gRelease(CCONTEXTMENU);
	
	if(m_aPidls && m_pMalloc)
		FreePidlTable();
	
	if(m_pMalloc)
		m_pMalloc->Release();


//	_RPTF1(_CRT_WARN, "~CContextMenu(%d)\n", g_DllRefCount);
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

STDMETHODIMP CContextMenu::QueryInterface(REFIID riid, 
										  LPVOID FAR *ppReturn)
{
	if(IsEqualIID(riid, IID_IUnknown))
	{
		*ppReturn = (LPUNKNOWN)(LPCONTEXTMENU)this;
		m_ObjRefCount++;
		return ResultFromScode(S_OK);
	}
	
	if(IsEqualIID(riid, IID_IContextMenu))
	{
		*ppReturn = (LPCONTEXTMENU)this;
		m_ObjRefCount++;
		return ResultFromScode(S_OK);
	}   
	
	if(IsEqualIID(riid, IID_IShellExtInit))
	{
		*ppReturn = (LPSHELLEXTINIT)this;
		m_ObjRefCount++;
		return ResultFromScode(S_OK);
	}   
	
	*ppReturn = NULL;
	return ResultFromScode(E_NOINTERFACE);
}                                             

STDMETHODIMP_(DWORD) CContextMenu::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CContextMenu::Release()
{
	if(--m_ObjRefCount == 0)
		delete this;
	
	return m_ObjRefCount;
}

///////////////////////////////////////////////////////////////////////////
//
// IContextMenu Implementation
//

/**************************************************************************
IContextMenu::GetCommandString
	Retrieves information about a shortcut menu command, including the help
	string and the language-independent, or canonical, name for the command.

Parameters
	idCmd		Menu command identifier offset.
	uFlags		Flags specifying the information to return. This parameter
				can have one of the following values:
        GCS_HELPTEXTA	Sets pszName to an ANSI string containing the help
						text for the command.
        GCS_HELPTEXTW	Sets pszName to a Unicode string containing the
						help text for the command.
        GCS_VALIDATEA	Returns S_OK if the menu item exists, or S_FALSE
						otherwise.
        GCS_VALIDATEW	Returns S_OK if the menu item exists, or S_FALSE
						otherwise.
        GCS_VERBA		Sets pszName to an ANSI string containing the
						language-independent command name for the menu item.
        GCS_VERBW		Sets pszName to a Unicode string containing the
						language-independent command name for the menu item.

	pwReserved	Reserved. Applications must specify NULL when calling this
				method, and handlers must ignore this parameter when called.
	pszName		Address of the buffer to receive the null-terminated string
				being retrieved.
	cchMax		Size of the buffer, in characters, to receive the
				null-terminated string.

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CContextMenu::GetCommandString(UINT idCommand,
											UINT uFlags,
											LPUINT lpReserved,
											LPSTR lpszName,
											UINT uMaxNameLen)
{
	HRESULT  hr = E_INVALIDARG;
	
	switch(uFlags)
	{
	case GCS_HELPTEXT:
		hr = E_NOTIMPL;
		break;
		
	case GCS_VERBA:
		switch(idCommand)
		{
		case IDM_RENAME:
			lstrcpyA((LPSTR)lpszName, "rename");
			hr = NOERROR;
			break;
		}
		break;
		
	//NT 4.0 with IE 3.0x or no IE will always call this with GCS_VERBW. In this 
	//case, you need to do the lstrcpyW to the pointer passed.
	case GCS_VERBW:
		switch(idCommand)
		{
		case IDM_RENAME:
			lstrcpyW((LPWSTR)lpszName, L"rename");
			hr = NOERROR;
			break;
		}
		break;
		
		case GCS_VALIDATE:
			hr = NOERROR;
			break;
	}
	
	return hr;
}

/**************************************************************************
IContextMenu::InvokeCommand
	Carries out the command associated with a shortcut menu item.

Parameters
	pici	Pointer to a CMINVOKECOMMANDINFO or CMINVOKECOMMANDINFOEX
			structure containing information about the command.
			For further details, see the Remarks section. 

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
	if(HIWORD(lpcmi->lpVerb))
	{
		//the command is being sent via a verb
		return NOERROR;
	}
	
	if(LOWORD(lpcmi->lpVerb) > IDM_LAST)
		return ResultFromScode(E_INVALIDARG);
	
	int i;

	switch(LOWORD(lpcmi->lpVerb))
	{
	case IDM_EXPLORE:
	case IDM_OPEN:
		// Find the first item in the list that is not a value.
		// These commands  should never be invoked if there isn't
		// at least one key item in the list.
		for(i=0; m_aPidls[i]; i++)
		{
			if( !CPidlManager::IsFile(m_aPidls[i]) )
				break;
		}
		OnOpenFolder(m_aPidls[i],
			lpcmi->hwnd,
			(LOWORD(lpcmi->lpVerb) == IDM_EXPLORE));
		break;
		
	case IDM_RENAME:
		break;
	
	case IDM_EXECUTE:
		for(i=0; m_aPidls[i]; i++)
		{
			if( CPidlManager::IsFile(m_aPidls[i]) )
				break;
		}
		OnExecute(m_aPidls[i]);
		break;
	
	case IDM_CREATE_FOLDER:
		OnCreateNewFolder( m_aPidls[0] );
		break;
	
	case IDM_REMOVE_FOLDER:
		if(NULL != m_aPidls[0])
			m_pSFParent->RemoveSubfolder(m_aPidls[0]);
		break;

	case IDM_PROPERTIES:
		if(NULL != m_aPidls[0])
			OnShowProperties(m_pSFParent->CreateFQPidl(m_aPidls[0]));
		else
			OnShowProperties(NULL);
		break;

	case IDM_ADDEXTFOLDER:
		if(NULL != m_aPidls[0])
			OnCreateExtensionFolder(m_aPidls[0]);
		break;

	case IDM_HIDEEXTENSION:
		if(NULL != m_aPidls[0])
			OnHideExtension(m_aPidls[0]);
		break;

	case IDM_SHOWONLYEXTENSION:
		if(NULL != m_aPidls[0])
			OnShowOnlyExtension(m_aPidls[0]);
		break;

	case IDM_OPENCONTAININGFOLDER1:
		if(NULL != m_aPidls[0])
			OnOpenContainingFolder(m_aPidls[0]);
		break;

	case IDM_CLEARSEARCH:
		OnClearSearch(m_aPidls[0]);
		break;
	}
	
	return NOERROR;
}

/**************************************************************************
IContextMenu::QueryContextMenu
	Adds commands to a shortcut menu.

Parameters
	hmenu		Handle to the menu. The handler should specify this handle
				when adding menu items. 
	indexMenu	Zero-based position at which to insert the first menu item. 
	idCmdFirst	Minimum value that the handler can specify for a menu item
				identifier.
	idCmdLast	Maximum value that the handler can specify for a menu item
				identifier.
	uFlags		Optional flags specifying how the shortcut menu can be
				changed. This parameter can be set to any combination of
				the following values. The remaining bits of the low-order
				word are reserved by the system. The high-order word can be
				used for context-specific communications. The CMF_RESERVED
				value can be used to mask out the low-order word.

        CMF_CANRENAME		This flag is set if the calling application
							supports renaming of items. A shortcut menu
							extension or drag-and-drop handler should ignore
							this flag. A namespace extension should add a
							rename item to the menu if applicable. 
        CMF_DEFAULTONLY		This flag is set when the user is activating the
							default action, typically by double-clicking.
							This flag provides a hint for the shortcut menu
							extension to add nothing if it does not modify
							the default item in the menu. A shortcut menu
							extension or drag-and-drop handler should not add
							any menu items if this value is specified.
							A namespace extension should add only the default
							item (if any). 
        CMF_DISABLEDVERBS	Microsoft Windows Vista and later:
        CMF_EXPLORE			This flag is set when the Microsoft Windows
							Explorer tree window is present.
        CMF_EXTENDEDVERBS	This flag is set when the calling application
							wants extended verbs. Normal verbs are displayed
							when the user right-clicks an object. To display
							extended verbs, the user must right-click while
							pressing the SHIFT key.
        CMF_INCLUDESTATIC	Deprecated, do not use.
        CMF_ITEMMENU		Microsoft Windows Vista and later: View specific.
        CMF_NODEFAULT		This flag is set if no item in the menu should be
							the default item. A drag-and-drop handler should
							ignore this flag. A namespace extension should not
							set any of the menu items to the default. 
        CMF_NORMAL			Indicates normal operation. A shortcut menu
							extension, namespace extension, or drag-and-drop
							handler can add all menu items. 
        CMF_NOVERBS			This flag is set for items displayed in the
							Send To menu. Shortcut menu handlers should
							ignore this value. 
        CMF_RESERVED		View specific.
        CMF_VERBSONLY		This flag is set if the shortcut menu is for a
							shortcut object. Shortcut menu handlers should
							ignore this value.

Return Value
	If successful, returns an HRESULT value that has its severity value
	set to SEVERITY_SUCCESS and its code value set to the offset of the
	largest command identifier that was assigned, plus one. For example,
	assume that idCmdFirst is set to 5 and you add three items to the
	menu with command identifiers of 5, 7, and 8. The return value should
	be MAKE_HRESULT(SEVERITY_SUCCESS, 0, 8 - 5 + 1). Otherwise, it returns
	an OLE error value.
**************************************************************************/
STDMETHODIMP CContextMenu::QueryContextMenu(HMENU hMenu,
											UINT indexMenu,
											UINT idCmdFirst,
											UINT idCmdLast,
											UINT uFlags)
{
	UINT	indexOrig = indexMenu;
	TCHAR	szText[100] = {0};
	UINT	nTextSize = ARRAYSIZE(szText);

	if(!(CMF_DEFAULTONLY & uFlags))
	{
		if(!m_fAllValues)
		{
			BOOL  fExplore = uFlags & CMF_EXPLORE;
			
			if(fExplore)
			{
				LoadString(g_hInst, IDS_EXPLORE, szText, nTextSize);
				AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_EXPLORE, TRUE, TRUE);

				LoadString(g_hInst, IDS_OPEN, szText, nTextSize);
				AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_OPEN, TRUE, FALSE);
			}
			else
			{
				LoadString(g_hInst, IDS_OPEN, szText, nTextSize);
				AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_OPEN, TRUE, TRUE);

				LoadString(g_hInst, IDS_EXPLORE, szText, nTextSize);
				AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_EXPLORE, TRUE, FALSE);
			}
			
			AddMenuItem(hMenu, NULL, indexMenu++); // Separator

			LoadString(g_hInst, IDS_REMOVE_FOLDER, szText, nTextSize);
			AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_REMOVE_FOLDER, CanRenameItems(), FALSE);

			if(uFlags & CMF_CANRENAME)
			{
				LoadString(g_hInst, IDS_RENAME, szText, nTextSize);
				AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_RENAME, CanRenameItems(), FALSE);
			}

			if(NULL != m_aPidls[0])
			{
				AddMenuItem(hMenu, NULL, indexMenu++); // Separator

				LoadString(g_hInst, IDS_CLEARSEARCH, szText, nTextSize);
				AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_CLEARSEARCH, TRUE, FALSE);
			}
		}
		else
		{
			if(NULL != m_aPidls[0])
			{
				LoadString(g_hInst, IDS_OPEN, szText, nTextSize);
				AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_EXECUTE, TRUE, TRUE);

				LPPIDLDATA pData = CPidlManager::GetDataPointer(m_aPidls[0]);
				TCHAR txt[100] = {0};
				LPCTSTR ext = CString::GetExtension(pData->fileData.szPath);
				if(ext)
				{
					int len = lstrlen(ext);
					if((len <= 5) && (len != lstrlen(pData->fileData.szPath)))
					{
						ext++;

						LoadString(g_hInst, IDS_ADDEXTFOLDER, szText, nTextSize);
						wsprintf(txt, szText, ext, ext);
						AddMenuItem(hMenu, txt, indexMenu++, idCmdFirst+IDM_ADDEXTFOLDER, TRUE, FALSE);

						LoadString(g_hInst, IDS_HIDEEXTENSION, szText, nTextSize);
						wsprintf(txt, szText, ext);
						AddMenuItem(hMenu, txt, indexMenu++, idCmdFirst+IDM_HIDEEXTENSION, TRUE, FALSE);

						LoadString(g_hInst, IDS_SHOWONLYEXTENSION, szText, nTextSize);
						wsprintf(txt, szText, ext);
						AddMenuItem(hMenu, txt, indexMenu++, idCmdFirst+IDM_SHOWONLYEXTENSION, TRUE, FALSE);

						LoadString(g_hInst, IDS_OPENCONTAININGFOLDER, szText, nTextSize);
						wsprintf(txt, szText, ext);
						AddMenuItem(hMenu, txt, indexMenu++, idCmdFirst+IDM_OPENCONTAININGFOLDER1, TRUE, FALSE);
					}
				}
			}
		}

		if(NULL == m_aPidls[0])
		{
			if(indexMenu > indexOrig)
				AddMenuItem(hMenu, NULL, indexMenu++); // Separator

			LoadString(g_hInst, IDS_CREATE_FOLDER, szText, nTextSize);
			AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_CREATE_FOLDER, TRUE, FALSE);

			AddMenuItem(hMenu, NULL, indexMenu++); // Separator

			LoadString(g_hInst, IDS_CLEARSEARCH, szText, nTextSize);
			AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_CLEARSEARCH, TRUE, FALSE);
		}

		AddMenuItem(hMenu, NULL, indexMenu++); // Separator

		LoadString(g_hInst, IDS_PROPERTIES, szText, nTextSize);
		AddMenuItem(hMenu, szText, indexMenu++, idCmdFirst+IDM_PROPERTIES, TRUE, FALSE);

		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_LAST + 1));
	}
	
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
}

///////////////////////////////////////////////////////////////////////////
//
// Private functions
//

BOOL CContextMenu::AllocPidlTable(DWORD dwEntries)
{
	//add one for NULL terminator
	dwEntries++;
	
	m_aPidls = (LPITEMIDLIST*)m_pMalloc->Alloc(dwEntries * sizeof(LPITEMIDLIST));
	
	if(m_aPidls)
	{
		//set all of the entries to NULL
		ZeroMemory(m_aPidls, dwEntries * sizeof(LPITEMIDLIST));
	}
	
	return (m_aPidls != NULL);
}

void CContextMenu::FreePidlTable(void)
{
	if(m_aPidls && g_pPidlMgr)
	{
		int   i;
		for(i = 0; m_aPidls[i]; i++)
			g_pPidlMgr->Delete(m_aPidls[i]);
		
		m_pMalloc->Free(m_aPidls);
		
		m_aPidls = NULL;
	}
}

BOOL CContextMenu::FillPidlTable(LPCITEMIDLIST *aPidls, UINT uItemCount)
{
	if(m_aPidls && g_pPidlMgr)
	{
		UINT  i;
		for(i = 0; i < uItemCount; i++)
		{
			m_aPidls[i] = g_pPidlMgr->Copy(aPidls[i]);
		}
		return TRUE;
	}
	
	return FALSE;
}

BOOL CContextMenu::CanRenameItems(void)
{
	if(m_aPidls && g_pPidlMgr)
	{
		//get the number of items assigned to this object
		UINT  i;
		for(i = 0; m_aPidls[i]; i++){}
		
		//you can't rename more than one item at a time
		if(i > 1)
			return FALSE;
		
		DWORD dwAttributes = SFGAO_CANRENAME;
		m_pSFParent->GetAttributesOf(i, (LPCITEMIDLIST*)m_aPidls, &dwAttributes);
		
		return dwAttributes & SFGAO_CANRENAME;
	}
	
	return FALSE;
}

void CContextMenu::AddMenuItem(HMENU hMenu,
							   LPTSTR text,
							   UINT indexMenu,
							   UINT commandIndex,
							   BOOL bEnabled,
							   BOOL bDefault)
{
	MENUITEMINFO   mii;

	ZeroMemory(&mii, sizeof(mii));

	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_ID | MIIM_TYPE;
	mii.fState = MFS_ENABLED;

	if(NULL == text)
	{
		mii.wID = 0;
		mii.fType = MFT_SEPARATOR;
	}
	else
	{
		mii.fMask |= MIIM_STATE;
		mii.wID = commandIndex;
		mii.fType = MFT_STRING;
		mii.dwTypeData = text;
		mii.fState = bEnabled ? MFS_ENABLED : MFS_DISABLED;
		if(bDefault)
			mii.fState |= MFS_DEFAULT;
	}

	InsertMenuItem(hMenu, indexMenu, TRUE, &mii);
}

void CContextMenu::OnExecute(LPCITEMIDLIST pidl)
{
/*
	LPPIDLDATA pData = CPidlManager::GetDataPointer(pidl);
	SHELLEXECUTEINFO  sei;
	ZeroMemory(&sei, sizeof(sei));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.lpFile = pData->fileData.szPath;
	sei.nShow = SW_SHOWNORMAL;//SW_SHOWDEFAULT;//SW_MAXIMIZE;
	ShellExecuteEx(&sei);
*/
	LPPIDLDATA pData = CPidlManager::GetDataPointer(pidl);
	//CPidl cpidl(pidl);
	//LPPIDLDATA pData = cpidl.GetData();
	if(pData->fileData.pidlFS)
	{
		SHELLEXECUTEINFO  sei;
		ZeroMemory(&sei, sizeof(sei));
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_IDLIST; 
		sei.lpIDList  = pData->fileData.pidlFS;
		sei.nShow = SW_SHOWNORMAL;
		ShellExecuteEx(&sei);
	}
}

void CContextMenu::OnOpenFolder(LPCITEMIDLIST pidl, HWND hwnd, bool bExplore)
{
	LPITEMIDLIST  pidlFQ;
	SHELLEXECUTEINFO  sei;

	pidlFQ = m_pSFParent->CreateFQPidl(pidl);

//	TRACE_PIDL_PATH("CContextMenu::OnOpenFolder(%s)\n", pidlFQ);

	ZeroMemory(&sei, sizeof(sei));

	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
	sei.lpIDList = pidlFQ;
	sei.lpClass = TEXT("folder");
	sei.hwnd = hwnd;
	sei.nShow = SW_SHOWNORMAL;
	sei.lpVerb = bExplore ? TEXT("explore") : TEXT("open");

	ShellExecuteEx(&sei);

	g_pPidlMgr->Delete(pidlFQ);
}

void CContextMenu::OnShowProperties(LPCITEMIDLIST pidl)
{
	CPidl pidlTemp(pidl);
	if( (NULL == pidl) || !pidlTemp.IsFile() )
	{
		/*LPPIDLDATA pData = CPidlManager::GetDataPointer(pidl);
		SHELLEXECUTEINFO  sei;
		ZeroMemory(&sei, sizeof(sei));
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.lpIDList = pidl;
		sei.nShow = SW_SHOW;
		sei.fMask = SEE_MASK_INVOKEIDLIST;
		sei.lpVerb = "properties";
		ShellExecuteEx(&sei);*/
		m_pSFParent->ShowProperties( pidl );
		return;
	}
	else
	{
		LPPIDLDATA pData = pidlTemp.GetData();
		SHELLEXECUTEINFO  sei;
		ZeroMemory(&sei, sizeof(sei));
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.lpFile = pData->fileData.szPath;
		sei.nShow = SW_SHOW;
		sei.fMask = SEE_MASK_INVOKEIDLIST;
		sei.lpVerb = "properties";
		ShellExecuteEx(&sei);
	}
}

void CContextMenu::OnCreateExtensionFolder(LPCITEMIDLIST pidl)
{
	// Get file extension
	LPPIDLDATA pData = CPidlManager::GetDataPointer(pidl);
	LPCTSTR ext = CString::GetExtension(pData->fileData.szPath);
	ext++;

	// Add new folder
	m_pSFParent->AddExtensionSubfolder( ext );

	// Add '-filetype:...' to the parent folder's query
	m_pSFParent->RemoveFiletype( ext );

	// Refresh the list view
	g_pViewList->Refresh();
}

void CContextMenu::OnHideExtension(LPCITEMIDLIST pidl)
{
	// Get file extension
	LPPIDLDATA pData = CPidlManager::GetDataPointer(pidl);
	LPCTSTR ext = CString::GetExtension(pData->fileData.szPath);
	ext++;

	// Add '-filetype:...' to the parent folder's query
	m_pSFParent->RemoveFiletype(ext);

	// Refresh the list view
	g_pViewList->Refresh();
}

void CContextMenu::OnCreateNewFolder(LPCITEMIDLIST pidl)
{
	TCHAR	szText[100] = {0};
	UINT	nTextSize = ARRAYSIZE(szText);
	LoadString(g_hInst, IDS_NEWFOLDER, szText, nTextSize);
	CPidl pidlNew = m_pSFParent->CreateSubfolder(pidl, szText);
}

void CContextMenu::OnShowOnlyExtension(LPCITEMIDLIST pidl)
{
	// Get file extension
	LPPIDLDATA pData = CPidlManager::GetDataPointer(pidl);
	LPCTSTR ext = CString::GetExtension(pData->fileData.szPath);
	ext++;

	m_pSFParent->ShowOnlyExtension(ext);

	// Refresh the list view
	g_pViewList->Refresh();
}

void CContextMenu::OnOpenContainingFolder(LPCITEMIDLIST pidl)
{
	/*CPidl cpidl(pidl);
	if( cpidl.IsFile() )
	{
		LPPIDLDATA pData = cpidl.GetData();
		LPFILEDATA pFileData = &pData->fileData;

		ZeroMemory(&sei, sizeof(sei));

		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
		sei.lpIDList = pFileData->pidlFS;
		sei.lpClass = TEXT("folder");
		sei.hwnd = hwnd;
		sei.nShow = SW_SHOWNORMAL;
		sei.lpVerb = TEXT("open");

		ShellExecuteEx(&sei);
	}*/
}

void CContextMenu::OnClearSearch(LPCITEMIDLIST pidl)
{
	m_pSFParent->ClearSearch(pidl);
}
