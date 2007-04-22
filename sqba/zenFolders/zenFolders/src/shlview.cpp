
#include <crtdbg.h>
//#include <windows.h>
//#include <commctrl.h>

#include "shlview.h"
#include "commands.h"
#include "zenfolders.h"
#include "guid.h"
#include "resource.h"
#include "dataobj.h"
#include "dropsrc.h"
//#include "icons.h"
#include "viewlist.h"
#include "cfgxml.h"
#include "statusbar.h"
#include "toolbar.h"
#include "util/string.h"
#include "util/string.h"
#include "util/settings.h"


//#define USE_ADSENSE


//#define IDM_VIEW_KEYS	(FCIDM_SHVIEWFIRST + 0x500)
//#define IDM_VIEW_IDW	(FCIDM_SHVIEWFIRST + 0x501)
//#define IDM_MYFILEITEM	(FCIDM_SHVIEWFIRST + 0x502)
#define NS_CLASS_NAME	(TEXT("zenFoldersNSClass"))
//#define ID_LISTVIEW		2000
#define MENU_OFFSET		1
#define MENU_MAX		100


//extern LPICONS		g_pIcons;
extern HINSTANCE	g_hInst;
extern LPVIEWSLIST	g_pViewList;
extern LPCONFIGXML	g_pConfigXML;
extern LPPIDLMGR	g_pPidlMgr;

int		g_nColumn1;
int		g_nColumn2;
LONG	g_lViewStyle = LVS_REPORT;
bool	g_columnOrders[] = { true, true };


int CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int result = 0;
	int column = 0;

	if(NULL != lParamSort)
		column = (int)lParamSort;

	bool descending = g_columnOrders[column];

	LPITEMIDLIST pidl1 = (LPITEMIDLIST)lParam1;
	LPITEMIDLIST pidl2 = (LPITEMIDLIST)lParam2;

	LPCTSTR val1, val2;

	if(CPidlManager::IsFile(pidl1) == CPidlManager::IsFile(pidl2))
	{
		LPPIDLDATA pData1 = CPidlManager::GetDataPointer(pidl1);
		LPPIDLDATA pData2 = CPidlManager::GetDataPointer(pidl2);

		if( CPidlManager::IsFile(pidl1) )
		{
			// Files
			if(column == 0)
			{
				val1 = descending ? pData1->szName : pData2->szName;
				val2 = descending ? pData2->szName : pData1->szName;
			}
			else
			{
				LPFILEDATA pFileData1 = &pData1->fileData;
				LPFILEDATA pFileData2 = &pData2->fileData;
				val1 = descending ? pFileData1->szPath : pFileData2->szPath;
				val2 = descending ? pFileData2->szPath : pFileData1->szPath;
			}
		}
		else
		{
			// Folders
			if(column == 0)
			{
				val1 = descending ? pData1->szName : pData2->szName;
				val2 = descending ? pData2->szName : pData1->szName;
			}
			else
			{
				LPFOLDERDATA pFolderData1 = &pData1->folderData;
				LPFOLDERDATA pFolderData2 = &pData2->folderData;
				val1 = descending ? pFolderData1->szQuery : pFolderData2->szQuery;
				val2 = descending ? pFolderData2->szQuery : pFolderData1->szQuery;
			}
		}
		result = lstrcmp(val1, val2);
	}
	else
	{
		if( !CPidlManager::IsFile(pidl1) )
			result = -1;	// Folders go before files
		else
			result = 1;		// and files go after folders
	}

	return result;
} 


CShellView::CShellView(CShellFolder *pFolder, LPCITEMIDLIST pidl)
{
#ifdef INITCOMMONCONTROLSEX
	INITCOMMONCONTROLSEX iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_LISTVIEW_CLASSES;
	::InitCommonControlsEx(&iccex);
#else
	::InitCommonControls();
#endif   //INITCOMMONCONTROLSEX
	
	m_hMenu				= NULL;
	m_pShellBrowser		= NULL;
	m_pCommDlgBrowser	= NULL;
	m_pListView			= NULL;
	m_pWebBrowser		= NULL;
	m_bInEdit			= FALSE;
	m_iLastSelectedItem	= -1;

	m_hAccels = ::LoadAccelerators(g_hInst, MAKEINTRESOURCE(IDR_ACCELERATORS));

	m_pSFParent = pFolder;
	if(m_pSFParent)
		m_pSFParent->AddRef();
	
	if(FAILED(::SHGetMalloc(&m_pMalloc)))
	{
		_RPTF0(_CRT_ERROR, "SHGetMalloc failed\n");
		delete this;
		return;
	}
	
	m_pidl = g_pPidlMgr->Copy(pidl);
	
	m_uState = SVUIA_DEACTIVATE;

	if(g_pViewList)
		g_pViewList->AddToList(this);
	
	m_ObjRefCount = 1;
	gAddRef(CSHELLVIEW);
}

CShellView::~CShellView()
{
	if(g_pViewList)
		g_pViewList->RemoveFromList(this);

	if(m_pidl)
	{
		g_pPidlMgr->Delete(m_pidl);
		m_pidl = NULL;
	}
	
	if(m_pSFParent)
		m_pSFParent->Release();
	
	if(m_pMalloc)
		m_pMalloc->Release();

	if(m_pCommDlgBrowser)
		m_pCommDlgBrowser->Release();

	if(m_pListView)
		delete m_pListView;

	if(m_pWebBrowser)
		delete m_pWebBrowser;
	
	gRelease(CSHELLVIEW);

//	_RPTF1(_CRT_WARN, "~CShellView(), g_DllRefCount=%d\n", g_DllRefCount);
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

STDMETHODIMP CShellView::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;
	
	//IUnknown
	if(IsEqualIID(riid, IID_IUnknown))
		*ppReturn = this;
	
	//IOleWindow
	else if(IsEqualIID(riid, IID_IOleWindow))
		*ppReturn = (IOleWindow*)this;
	
	//IShellView
	else if(IsEqualIID(riid, IID_IShellView))
		*ppReturn = (IShellView*)this;
	
	//IOleCommandTarget
	else if(IsEqualIID(riid, IID_IOleCommandTarget))
		*ppReturn = (IOleCommandTarget*)this;
	
	if(*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CShellView::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CShellView::Release()
{
	if(--m_ObjRefCount == 0)
	{
		delete this;
		return 0;
	}
	
	return m_ObjRefCount;
}

///////////////////////////////////////////////////////////////////////////
//
// IOleWindow Implementation
//

STDMETHODIMP CShellView::GetWindow(HWND *phWnd)
{
	*phWnd = m_hWnd;
	
	return S_OK;
}

STDMETHODIMP CShellView::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////
//
// IOleCommandTarget Implementation
//

/**************************************************************************
IOleCommandTarget::QueryStatus
	Queries the object for the status of one or more commands generated by
	user interface events.

Parameters
	pguidCmdGroup [unique][in]	Unique identifier of the command group;
								can be NULL to specify the standard group.
								All the commands that are passed in the
								prgCmds array must belong to the group
								specified by pguidCmdGroup.
	cCmds [in]					The number of commands in the prgCmds array.
	prgCmds [in,out]			A caller-allocated array of OLECMD structures
								that indicate the commands for which the
								caller needs status information. This method
								fills the cmdf member of each structure with
								values taken from the OLECMDF enumeration.
	pCmdText [unique][in,out]	Pointer to an OLECMDTEXT structure in which
								to return name and/or status information of
								a single command. Can be NULL to indicate
								that the caller does not need this information.

Return Value
	This method supports the standard return values E_FAIL and E_UNEXPECTED,
	as well as the following:
	S_OK						The command status as any optional strings
								were returned successfully.
	E_POINTER					The prgCmds argument is NULL.
	OLECMDERR_E_UNKNOWNGROUP	The pguidCmdGroup parameter is not NULL
								but does not specify a recognized command
								group.
**************************************************************************/
STDMETHODIMP CShellView::QueryStatus(const GUID *pguidCmdGroup, 
									 ULONG cCmds, 
									 OLECMD prgCmds[], 
									 OLECMDTEXT *pCmdText)
{
	ULONG i;
	
	for(i = 0; i < cCmds;i++)
	{
		prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
		//_RPTF1(_CRT_WARN, "QueryStatus %d\n", prgCmds[i].cmdID);
		/*switch(prgCmds[i].cmdID)
		{
		case OLECMDID_COPY:
			prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
			break;
		case OLECMDID_OPEN:
			prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
			break;
		}*/
	}
	
	return S_OK;

/*
	//only process the commands for our command group
	if(pguidCmdGroup && (*pguidCmdGroup != CLSID_CmdGrp))
		return OLECMDERR_E_UNKNOWNGROUP;
	
	//make sure prgCmds is not NULL
	if(!prgCmds)
		return E_POINTER;
	
	//run through all of the commands and supply the correct information
	for(i = 0; i < cCmds;i++)
	{
		switch(prgCmds[i].cmdID)
		{
		case IDM_VIEW_KEYS:
			prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
			break;
		}
	}
	
	return S_OK;
*/
}

/**************************************************************************
IOleCommandTarget::Exec
	Executes a specified command or displays help for a command.

Parameters
	pguidCmdGroup [unique][in]	Pointer to unique identifier of the command
								group; can be NULL to specify the standard
								group.
	nCmdID [in]					The command to be executed. This command
								must belong to the group specified with
								pguidCmdGroup.
	nCmdExecOpt [in]			Values taken from the OLECMDEXECOPT
								enumeration, which describe how the object
								should execute the command.
	pvaIn [unique][in]			Pointer to a VARIANTARG structure
								containing input arguments. Can be NULL.
	pvaOut [unique][in,out]		Pointer to a VARIANTARG structure to receive
								command output. Can be NULL.

Return Value
	This method supports the standard return values E_FAIL and E_UNEXPECTED,
	as well as the following:
	S_OK						The command was executed successfully.
	OLECMDERR_E_UNKNOWNGROUP	The pguidCmdGroup parameter is not NULL
								but does not specify a recognized command
								group.
	OLECMDERR_E_NOTSUPPORTED	The nCmdID parameter is not a valid command
								in the group identified by pguidCmdGroup.
	OLECMDERR_E_DISABLED		The command identified by nCmdID is currently
								disabled and cannot be executed.
	OLECMDERR_E_NOHELP			The caller has asked for help on the command
								identified by nCmdID, but no help is available.
	OLECMDERR_E_CANCELED		The user canceled the execution of the command.
**************************************************************************/
STDMETHODIMP CShellView::Exec(const GUID *pguidCmdGroup, 
							  DWORD nCmdID, 
							  DWORD nCmdExecOpt, 
							  VARIANTARG *pvaIn, 
							  VARIANTARG *pvaOut)
{
	if ( NULL == pguidCmdGroup )
    {
		if ( OLECMDID_OPEN == nCmdID )
        {
			_RPTF0(_CRT_WARN, "Open()\n");
			return S_OK;
        }
    }

//	_RPTF0(_CRT_WARN, "CShellView::Exec\n");
	//only process the commands for our command group
	if(pguidCmdGroup && (*pguidCmdGroup == CLSID_CmdGrp))
	{
//		_RPTF0(_CRT_WARN, "CShellView::Exec -> OnCommand()\n");
		OnCommand(nCmdID, 0, NULL);
		return S_OK;
	}
	
	return OLECMDERR_E_UNKNOWNGROUP;
}

///////////////////////////////////////////////////////////////////////////
//
// IShellView Implementation
//

/**************************************************************************
IShellView::AddPropertySheetPages
	Allows the view to add pages to the Options property sheet from the
	View menu.

Parameters
	dwReserved [in]	Reserved. 
	lpfn [in]		Address of the callback function used to add the pages. 
	lparam [in]		Value that must be passed as the callback function's
					lparam parameter. 

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CShellView::AddPropertySheetPages(DWORD dwReserved, 
											   LPFNADDPROPSHEETPAGE lpfn, 
											   LPARAM lParam)
{
	return E_NOTIMPL;
}

/**************************************************************************
IShellView::CreateViewWindow
	Creates a view window. This can be either the right pane of
	Microsoft Windows Explorer or the client window of a folder window.

Parameters
	psvPrevious [in]	Address of the IShellView interface of the view
						window being exited. Views can use this parameter
						to communicate with a previous view of the same
						implementation. This interface can be used to
						optimize browsing between like views. This pointer
						may be NULL. 
	pfs [in]			Address of a FOLDERSETTINGS structure. The view
						should use this when creating its view. 
	psb [in]			Address of the current instance of the IShellBrowser
						interface. The view should call this interface's
						AddRef method and keep the interface pointer to
						allow communication with the Windows Explorer window. 
	prcView [in]		Dimensions of the new view, in client coordinates. 
	phWnd [out]			Address of the window handle being created. 

Return Value
	Returns an OLE success code if successful, or an OLE error code
	otherwise. Use the SUCCEEDED and FAILED macros to determine whether
	the operation succeeded or failed.
**************************************************************************/
STDMETHODIMP CShellView::CreateViewWindow(LPSHELLVIEW pPrevView, 
										  LPCFOLDERSETTINGS lpfs, 
										  LPSHELLBROWSER psb, 
										  LPRECT prcView, 
										  HWND *phWnd)
{
	WNDCLASS wc = {0};

	*phWnd = NULL;

	//if our window class has not been registered, then do so
	if( !GetClassInfo(g_hInst, NS_CLASS_NAME, &wc) )
	{
		ZeroMemory(&wc, sizeof(wc));

		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= g_hInst;
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= NS_CLASS_NAME;

		if(!RegisterClass(&wc))
			return E_FAIL;
	}
	
	//set up the member variables
	m_pShellBrowser = psb;
	m_FolderSettings = *lpfs;

	//get our parent window
	m_pShellBrowser->GetWindow(&m_hwndParent);
	
	m_pShellBrowser->QueryInterface(
		IID_ICommDlgBrowser,
		(LPVOID*)&m_pCommDlgBrowser);
	
	*phWnd = CreateWindowEx(
		0,
		NS_CLASS_NAME,
		NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		prcView->left,
		prcView->top,
		prcView->right - prcView->left,
		prcView->bottom - prcView->top,
		m_hwndParent,
		NULL,
		g_hInst,
		(LPVOID)this);
	
	if(!*phWnd)
		return E_FAIL;

	CToolBar::MergeToolbar( m_pShellBrowser );
	
	m_pShellBrowser->AddRef();

	return S_OK;
}

/**************************************************************************
IShellView::DestroyViewWindow
	Destroys the view window.

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CShellView::DestroyViewWindow(void)
{
	//Make absolutely sure all our UI is cleaned up.
	UIActivate(SVUIA_DEACTIVATE);
	
	if(m_hMenu)
		DestroyMenu(m_hMenu);
	
	DestroyWindow(m_hWnd);
	
	//release the shell browser object
	m_pShellBrowser->Release();
	
	return S_OK;
}

/**************************************************************************
IShellView::EnableModeless
	Enables or disables modeless dialog boxes. This method is not
	currently implemented.

Parameters
	fEnable	Nonzero to enable modeless dialog box windows or zero to
			disable them. 

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CShellView::EnableModeless(BOOL fEnable)
{
	return E_NOTIMPL;
}

/**************************************************************************
IShellView::GetCurrentInfo
	Retrieves the current folder settings.
	
Parameters
	lpfs	Address of a FOLDERSETTINGS structure to receive the settings. 

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CShellView::GetCurrentInfo(LPFOLDERSETTINGS lpfs)
{
	*lpfs = m_FolderSettings;
	
	return S_OK;
}

/**************************************************************************
IShellView::GetItemObject
	Retrieves an interface that refers to data presented in the view.

Parameters
	uItem	Constants that refer to an aspect of the view. This parameter
			can be any of the following values from the SVGIO enumeration.:
			SVGIO_BACKGROUND	Refers to the background of the view.
								It is used with IID_IContextMenu to get a
								shortcut menu for the view background. 
			SVGIO_SELECTION		Refers to the currently selected items.
								IID_IDataObject uses this constant to
								retrieve a data object that represents the
								selected items. 
			SVGIO_ALLVIEW		Same as SVGIO_SELECTION but refers to all
								items in the view. 
			SVGIO_CHECKED		Used in the same way as SVGIO_SELECTION
								but refers to checked items in views where
								checked mode is supported.
			SVGIO_TYPE_MASK		Masks all bits but those corresponding to
								the SVGIO flags.
			SVGIO_FLAG_VIEWORDER	Return the items in the order that they
									appear in the view. If this flag is not
									set, the selected item will be listed first.

    riid	Identifier of the Component Object Model (COM) interface
			being requested. 
    ppv		Address that receives the interface pointer. If an error occurs,
			the pointer returned must be NULL. 

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CShellView::GetItemObject(UINT uItem, REFIID riid, LPVOID *ppvOut)
{
	HRESULT  hr = E_NOINTERFACE;
	
	*ppvOut = NULL;
	
	if((SVGIO_BACKGROUND == uItem) && IsEqualIID(riid, IID_IContextMenu))
	{
		return m_pSFParent->CreateViewObject(NULL, riid, ppvOut);
	}
	else if(IsEqualIID(riid, IID_IDataObject))
	{
		LPITEMIDLIST   *pPidls;
		int            nCount;
		
		switch(uItem)
		{
		case SVGIO_SELECTION:
			nCount = m_pListView->GetSelectedCount();
			pPidls = (LPITEMIDLIST*)m_pMalloc->Alloc(nCount * sizeof(LPITEMIDLIST));
			if(pPidls)
            {
				UINT     i;
				LVITEM   lvItem;
				
				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = -1;
				lvItem.iSubItem = 0;
				
				for(i = 0; -1 != (lvItem.iItem = m_pListView->GetNextItem(lvItem.iItem, LVNI_SELECTED)); i++)
				{
					if(m_pListView->GetItem(&lvItem))
					{
						pPidls[i] = (LPITEMIDLIST)lvItem.lParam;
					}
				}
            }
			break;
			
		case SVGIO_ALLVIEW:
			nCount = m_pListView->GetItemCount();
			pPidls = (LPITEMIDLIST*)m_pMalloc->Alloc(nCount * sizeof(LPITEMIDLIST));
			if(pPidls)
            {
				LVITEM   lvItem;
				
				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = 0;
				lvItem.iSubItem = 0;
				
				for(lvItem.iItem = 0; lvItem.iItem < nCount; lvItem.iItem++)
				{
					if(m_pListView->GetItem(&lvItem))
					{
						pPidls[lvItem.iItem] = (LPITEMIDLIST)lvItem.lParam;
					}
				}
            }
			break;
			
		default:
			pPidls = NULL;
			nCount = 0;
			break;
		}
		
		if(pPidls)
		{
			CDataObject *pdo = new CDataObject(m_pSFParent, (LPCITEMIDLIST*)pPidls, nCount);
			
			if(pdo)
			{
				*ppvOut = pdo;
				hr = S_OK;
			}
			
			m_pMalloc->Free(pPidls);
		}
	}
	
	return hr;
}

/**************************************************************************
IShellView::Refresh
	Refreshes the view's contents in response to user input.

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CShellView::Refresh(void)
{
	if(m_pListView)
		m_pListView->Clear();
	
	FillList();
	
	return S_OK;
}

/**************************************************************************
IShellView::SaveViewState
	Saves the Shell's view settings so the current state can be restored
	during a subsequent browsing session.

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CShellView::SaveViewState(void)
{
	return S_OK;
}

/**************************************************************************
IShellView::SelectItem
	Changes the selection state of one or more items within the
	Shell view window.

Parameters
	pidlItem	Address of the ITEMIDLIST structure. 
	uFlags		One of the SVSIF constants that specify the type of
				selection to apply.

Return Value
	Returns S_OK if successful, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellView::SelectItem(LPCITEMIDLIST pidlItem, UINT uFlags)
{
	return E_NOTIMPL;
}

/**************************************************************************
IShellView::TranslateAccelerator
	Translates accelerator key strokes when a namespace extension's view
	has the focus.

Parameters
	lpmsg	Address of the message to be translated. 

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
	If the view returns S_OK, it indicates that the message was translated
	and should not be translated or dispatched by Microsoft Windows Explorer.
**************************************************************************/
STDMETHODIMP CShellView::TranslateAccelerator(LPMSG pMsg)
{
	if(m_bInEdit)
	{
		if((pMsg->message >= WM_KEYFIRST) && (pMsg->message <= WM_KEYLAST))
		{
			TranslateMessage(pMsg);
			DispatchMessage(pMsg);
			return S_OK;
		}
	}
	else if(::TranslateAccelerator(m_hWnd, m_hAccels, pMsg))
		return S_OK;
	
	return S_FALSE;
}

/**************************************************************************
IShellView::UIActivate
	Called when the activation state of the view window is changed by an
	event that is not caused by the Shell view itself. For example, if the
	TAB key is pressed when the tree has the focus, the view should be
	given the focus.

Parameters
	uState	Flag specifying the activation state of the window.
			This parameter can be one of the following values:

			SVUIA_ACTIVATE_FOCUS	Microsoft Windows Explorer has just
									created the view window with the input
									focus. This means the Shell view should
									be able to set menu items appropriate
									for the focused state.
			SVUIA_ACTIVATE_NOFOCUS	The Shell view is losing the input focus,
									or it has just been created without the
									input focus. The Shell view should be
									able to set menu items appropriate for
									the nonfocused state. This means no
									selection-specific items should be added. 
			SVUIA_DEACTIVATE		Windows Explorer is about to destroy the
									Shell view window. The Shell view should
									remove all extended user interfaces.
									These are typically merged menus and
									merged modeless pop-up windows. 
			SVUIA_INPLACEACTIVATE	The Shell view is active without focus.
									This flag is only used when UIActivate is
									exposed through the IShellView2 interface. 

Return Value
	Returns NOERROR if successful, or an OLE-defined error value otherwise.
**************************************************************************/
STDMETHODIMP CShellView::UIActivate(UINT uState)
{
	//don't do anything if the state isn't really changing
	if(m_uState == uState)
		return S_OK;
	
	//OnActivate handles the menu merging and internal state
	OnActivate(uState);
	
	//only do this if we are active
	if(uState != SVUIA_DEACTIVATE)
	{
		CStatusBar::Init(m_pShellBrowser);
	}
	
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
// Private functions
//

LRESULT CALLBACK CShellView::WndProc(HWND hWnd,
									 UINT uMessage,
									 WPARAM wParam,
									 LPARAM lParam)
{
	CShellView  *pThis = (CShellView*)GetWindowLong(hWnd, GWL_USERDATA);
	
	switch (uMessage)
	{
	case WM_NCCREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			pThis = (CShellView*)(lpcs->lpCreateParams);
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)pThis);
			
			//set the window handle
			pThis->m_hWnd = hWnd;
		}
		break;
		
	case WM_SIZE:
		return pThis->OnSize(LOWORD(lParam), HIWORD(lParam));
		
	case WM_CREATE:
		return pThis->OnCreate();
		
	case WM_SETFOCUS:
		return pThis->OnSetFocus();
		
	case WM_KILLFOCUS:
		return pThis->OnKillFocus();
		
	case WM_ACTIVATE:
		return pThis->OnActivate(SVUIA_ACTIVATE_FOCUS);
		
	case WM_COMMAND:
		return pThis->OnCommand(wParam, lParam);
		
	case WM_INITMENUPOPUP:
		return pThis->OnInitMenuPopup((HMENU)wParam);
		
	case WM_NOTIFY:
		return pThis->OnNotify(wParam, lParam);
		
	case WM_SETTINGCHANGE:
		return pThis->OnSettingChange((LPCTSTR)lParam);
	}
	
	return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

//#if (_WIN32_IE >= 0x0400)
typedef void (WINAPI *PFNSHGETSETTINGSPROC)(LPSHELLFLAGSTATE lpsfs, DWORD dwMask);
//#endif   //(_WIN32_IE >= 0x0400)

void CShellView::UpdateShellSettings(void)
{
//#if (_WIN32_IE >= 0x0400)
	SHELLFLAGSTATE       sfs = {0};
	HINSTANCE            hinstShell32;
	
	//Since SHGetSettings is not implemented in all versions of the shell, get the 
	//function address manually at run time. This allows the extension to run on all 
	//platforms.
	
	//ZeroMemory(&sfs, sizeof(sfs));
	
	//The default, in case any of the following steps fails, is classic Windows 95 
	//style.
	sfs.fWin95Classic = TRUE;
	
	hinstShell32 = LoadLibrary(TEXT("shell32.dll"));
	if(hinstShell32)
	{
		PFNSHGETSETTINGSPROC pfnSHGetSettings;
		
		pfnSHGetSettings = (PFNSHGETSETTINGSPROC)GetProcAddress(hinstShell32, "SHGetSettings");
		if(pfnSHGetSettings)
		{
			(*pfnSHGetSettings)(&sfs, SSF_DOUBLECLICKINWEBVIEW | SSF_WIN95CLASSIC);
		}
		FreeLibrary(hinstShell32);
	}
/*
	if(m_pListView)
	{
		DWORD dwExStyles = 0;
		
		if(!sfs.fWin95Classic && !sfs.fDoubleClickInWebView)
			dwExStyles |= LVS_EX_ONECLICKACTIVATE | 
			LVS_EX_TRACKSELECT | 
			LVS_EX_UNDERLINEHOT;

		dwExStyles |= LVS_EX_FULLROWSELECT;
		
		if(LVS_REPORT2 == m_pListView->GetStyle())
			m_pListView->SetExtendedListViewStyle( dwExStyles );
	}
*/
//#endif   //(_WIN32_IE >= 0x0400)
}

LRESULT CShellView::OnSetFocus(void)
{
	//Tell the browser one of our windows has received the focus.
	//This should always be done before merging menus
	//(OnActivate merges the menus) if one of our 
	//windows has the focus.
	m_pShellBrowser->OnViewWindowActive(this);
	
	OnActivate(SVUIA_ACTIVATE_FOCUS);

	StateChange(CDBOSC_SETFOCUS);

	return 0;
}

LRESULT CShellView::OnKillFocus(void)
{
	OnActivate(SVUIA_ACTIVATE_NOFOCUS);

	StateChange(CDBOSC_KILLFOCUS);

	return 0;
}

void CShellView::OnDeleteItem(LPNMHDR lpnmh)
{
	NM_LISTVIEW *lpnmlv = (NM_LISTVIEW*)lpnmh;
	
	//delete the pidl because we made a copy of it
	g_pPidlMgr->Delete((LPITEMIDLIST)lpnmlv->lParam);
}

LRESULT CShellView::OnNotify(WPARAM wParam, LPARAM lParam)
{
	UINT CtlID = (UINT)wParam;
	LPNMHDR lpnmh = (LPNMHDR)lParam;

	switch(lpnmh->code)
	{
	case TTN_NEEDTEXTA:
		if( CToolBar::OnTtnNeedTextA(lpnmh) )
			return TRUE;
		break;
		
	case TTN_NEEDTEXTW:
		if( CToolBar::OnTtnNeedTextW(lpnmh) )
			return TRUE;
		break;

	case NM_SETFOCUS:
		OnSetFocus();
		break;
		
	case NM_KILLFOCUS:
		OnDeactivate();
		break;
		
	case HDN_ENDTRACK:
		OnEndTrack();
		return 0;
		
	case LVN_DELETEITEM:
		OnDeleteItem(lpnmh);
		break;
		
#ifdef LVN_ITEMACTIVATE
	case LVN_ITEMACTIVATE:
#else    //LVN_ITEMACTIVATE
	case NM_DBLCLK:
	case NM_RETURN:
#endif   //LVN_ITEMACTIVATE
		OnDefaultAction();
		return 0;

	case LVN_ITEMCHANGED:
		OnItemChanged( (LPNMLISTVIEW)lpnmh );
		break;

	case LVN_GETDISPINFO:
		return OnGetDispInfo(lpnmh);

	case LVN_BEGINRDRAG:
	case LVN_BEGINDRAG:
		OnBeginDrag();
		return 0;
		
	case NM_RCLICK:
		return OnRightClick();

	case HDN_ITEMCLICK:
		OnHeaderClick( (LPNMHEADER)lpnmh );
		break;

	case LVN_BEGINLABELEDIT:
		m_bInEdit = TRUE;
		return OnBeginLabelEdit( (NMLVDISPINFO*)lpnmh );

	case LVN_ENDLABELEDIT:
		m_bInEdit = FALSE;
		OnEndLabelEdit( (NMLVDISPINFO*)lpnmh );
		break;

	case NM_CUSTOMDRAW:
		return m_pListView->OnCustomDraw(lParam);
		/*{
			LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
			switch(lplvcd->nmcd.dwDrawStage)
			{
			case CDDS_PREPAINT:
				//::SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (LONG_PTR)CDRF_NOTIFYITEMDRAW);
				return CDRF_NOTIFYITEMDRAW;
			case CDDS_ITEMPREPAINT:
				{
					int iRow = (int)lplvcd->nmcd.dwItemSpec;
					if(iRow%2 == 0)
					{
						// pListDraw->clrText   = RGB(252, 177, 0);
						lplvcd->clrTextBk = RGB(202, 221,250);
						return CDRF_NEWFONT;
					}
				}
			}

			//int w = LOWORD(wParam);
			//LPNMLISTVIEW pnm = (LPNMLISTVIEW)lParam;
			//if(pnm->hdr.hwndFrom == m_pListView->GetHwnd() && pnm->hdr.code == NM_CUSTOMDRAW)
			//{
			//	::SetWindowLong(hWnd, DWL_MSGRESULT, (LONG)CListView::TableDraw(lParam));
			//	return TRUE;
			//}
		}*/
		break;
	}

   return 0;
}

BOOL CShellView::OnBeginLabelEdit(NMLVDISPINFO *pdi)
{
	return FALSE;
/*
	LVITEM lvItem;
	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = pdi->item.iItem;
	lvItem.iSubItem = 0;
	if( m_pListView->GetItem(&lvItem) )
	{
		LPITEMIDLIST pidl = (LPITEMIDLIST)lvItem.lParam;
		return CPidlManager::IsFile(pidl);
	}
	return TRUE;
*/
}

void CShellView::OnEndLabelEdit(NMLVDISPINFO *pdi)
{
	if(NULL == pdi->item.pszText)
		return;

	if(0 == lstrlen(pdi->item.pszText))
		return;

//	_RPTF1(_CRT_WARN, "CShellView::OnEndLabelEdit(%s)\n", pdi->item.pszText);

	LVITEM lvItem;
	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = pdi->item.iItem;
	lvItem.iSubItem = 0;
	if( m_pListView->GetItem(&lvItem) )
	{
		LPITEMIDLIST pidl = (LPITEMIDLIST)lvItem.lParam;
		LPCTSTR pszName = pdi->item.pszText;
		if( m_pSFParent->Rename(pidl, pszName) )
		{
			int refresh_only_item;
			Refresh();
		}
	}
}

void CShellView::OnHeaderClick(LPNMHEADER lpnmh)
{
	int column = lpnmh->iItem;
	m_pListView->SortItems(SortFunc, (LPARAM)column);
	g_columnOrders[column] = !g_columnOrders[column];
	//ListView_SortItemsEx
}

int CShellView::OnBeginDrag()
{
	_RPTF0(_CRT_WARN, "CShellView::OnBeginDrag()\n");

	HRESULT        hr;
	IDataObject    *pDataObject = NULL;
	UINT           uItemCount;
	LPITEMIDLIST   *aPidls;
	
	uItemCount = m_pListView->GetSelectedCount();
	if(!uItemCount)
		return 0;
	
	aPidls = (LPITEMIDLIST*)m_pMalloc->Alloc(uItemCount * sizeof(LPITEMIDLIST));
	
	if(!aPidls)
		return 0;

	UINT i, x;
	
	for(i=0, x=0; x<uItemCount && i<m_pListView->GetItemCount(); i++)
	{
		if(m_pListView->GetItemState(i, LVIS_SELECTED))
		{
			LVITEM lvItem;
			
			lvItem.mask = LVIF_PARAM;
			lvItem.iItem = i;
			
			m_pListView->GetItem(&lvItem);
			aPidls[x] = (LPITEMIDLIST)lvItem.lParam;

			x++;
		}
	}
	
	hr = m_pSFParent->GetUIObjectOf(
		m_hWnd, 
		uItemCount, 
		(LPCITEMIDLIST*)aPidls, 
		IID_IDataObject, 
		NULL, 
		(LPVOID*)&pDataObject);
	
	if(SUCCEEDED(hr) && pDataObject)
	{
        IDropSource *pDropSource = new CDropSource;
        DWORD dwEffect = DROPEFFECT_COPY | DROPEFFECT_LINK | DROPEFFECT_MOVE;
        DWORD dwAttributes = SFGAO_CANLINK;
		/*
        hr = m_pSFParent->GetAttributesOf(  uItemCount, 
		(LPCITEMIDLIST*)aPidls, 
		&dwAttributes);
        
		  if(SUCCEEDED(hr) && (dwAttributes & SFGAO_CANLINK))
		  {
		  dwEffect |= DROPEFFECT_LINK;
		  }
		*/
        
        DoDragDrop(pDataObject, pDropSource, dwEffect, &dwEffect);
		
        pDataObject->Release();
        pDropSource->Release();
	}
	
	m_pMalloc->Free(aPidls);

	return 0;
}

int CShellView::OnRightClick()
{
	DWORD dwCursor = GetMessagePos();
	DoContextMenu(LOWORD(dwCursor), HIWORD(dwCursor), FALSE);
	return 0;
}

LRESULT CShellView::OnSize(WORD wWidth, WORD wHeight)
{

#ifdef USE_ADSENSE
	m_pWebBrowser->Move(0, 0, wWidth/3, wHeight, TRUE);
	m_pListView->Move(wWidth/3, 0, (wWidth/3)*2, wHeight, TRUE);
	return TRUE;
#else
	if(m_pListView)
		return m_pListView->Move(0, 0, wWidth, wHeight, TRUE);
	else if(m_pWebBrowser)
		return m_pWebBrowser->Move(0, 0, wWidth, wHeight, TRUE);
#endif
	return 0;
}

LRESULT CShellView::OnCreate(void)
{
	if( CreateList() )
	{
		if( InitList() )
		{
			FillList();
		}
		else
		{
			if( m_pWebBrowser )
			{
				m_pWebBrowser->Navigate( DOWNLOAD_PAGE );
			}
		}
	}
	
	return 0;
}

BOOL CShellView::CreateList(void)
{
	if( CGoogleDS::IsInstalled() )
	{
		m_pListView = new CListView(g_hInst, m_hWnd);
#ifdef USE_ADSENSE
		m_pWebBrowser = new CWebBrowser(g_hInst, m_hWnd);
#endif
	}
	else
	{
		m_pWebBrowser = new CWebBrowser(g_hInst, m_hWnd);
	}

	UpdateShellSettings();
	
	return TRUE;
}

LRESULT CShellView::OnActivate(UINT uState)
{
	//don't do anything if the state isn't really changing
	if(m_uState == uState)
		return S_OK;
	
	OnDeactivate();
	
	//only do this if we are active
	if(uState != SVUIA_DEACTIVATE)
	{
		//merge the menus
		m_hMenu = CreateMenu();
		
		if(m_hMenu)
		{
			InitMenu(uState);
		}
	}
	
	m_uState = uState;
	
	return 0;
}

void CShellView::OnDeactivate(void)
{
	if(m_uState != SVUIA_DEACTIVATE)
	{
		if(m_hMenu)
		{
			m_pShellBrowser->SetMenuSB(NULL, NULL, NULL);
			m_pShellBrowser->RemoveMenusSB(m_hMenu);
			DestroyMenu(m_hMenu);
			m_hMenu = NULL;
		}
		m_uState = SVUIA_DEACTIVATE;
	}
}

LRESULT CShellView::OnSettingChange(LPCTSTR lpszSection)
{
	//if(0 == lstrcmpi(lpszSection, TEXT("ShellState")))
	{
		UpdateShellSettings();
		return 0;
	}
	
	return 0;
}

void CShellView::DoContextMenu(WORD x, WORD y, BOOL fDefault)
{
	UINT           uSelected = m_pListView->GetSelectedCount();
	LPITEMIDLIST   *aSelectedItems;
	
	aSelectedItems = (LPITEMIDLIST*)m_pMalloc->Alloc(uSelected * sizeof(LPITEMIDLIST));
	
	if(aSelectedItems)
	{
		UINT           i;
		LPCONTEXTMENU  pContextMenu = NULL;
		LVITEM         lvItem = {0};
		
		//ZeroMemory(&lvItem, sizeof(lvItem));
		lvItem.mask = LVIF_STATE | LVIF_PARAM;
		lvItem.stateMask = LVIS_SELECTED;
		lvItem.iItem = 0;
		
		i = 0;
		
		while(m_pListView->GetItem(&lvItem) && (i < uSelected))
		{
			if(lvItem.state & LVIS_SELECTED)
				aSelectedItems[i++] = (LPITEMIDLIST)lvItem.lParam;
			lvItem.iItem++;
		}
		
		m_pSFParent->GetUIObjectOf(
			m_hwndParent,
			uSelected,
			(LPCITEMIDLIST*)aSelectedItems,
			IID_IContextMenu,
			NULL,
			(LPVOID*)&pContextMenu);
		
		if(pContextMenu)
		{
			HMENU hMenu = CreatePopupMenu();
			
			//See if we are in Explore or Open mode. If the browser's tree is present, 
			//then we are in Explore mode.
			BOOL  fExplore = FALSE;
			HWND  hwndTree = NULL;
			if(SUCCEEDED(m_pShellBrowser->GetControlWindow(FCW_TREE, &hwndTree)) && hwndTree)
			{
				fExplore = TRUE;
			}
			
			if(hMenu && SUCCEEDED(pContextMenu->QueryContextMenu(
				hMenu,
				0,
				MENU_OFFSET,
				MENU_MAX,
				CMF_NORMAL | 
				(uSelected != 1 ? 0 : CMF_CANRENAME) | 
				(fExplore ? CMF_EXPLORE : 0))))
			{
				UINT  uCommand;
				
				if(fDefault)
				{
					MENUITEMINFO   mii = {0};
					int            nMenuIndex;
					
					uCommand = 0;
					
					//ZeroMemory(&mii, sizeof(mii));
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_STATE | MIIM_ID;
					
					nMenuIndex = 0;
					
					//find the default item in the menu
					while(GetMenuItemInfo(hMenu, nMenuIndex, TRUE, &mii))
					{
						if(mii.fState & MFS_DEFAULT)
						{
							uCommand = mii.wID;
							break;
						}
						
						nMenuIndex++;
					}
				}
				else
				{
					uCommand = TrackPopupMenu(
						hMenu,
						TPM_LEFTALIGN | TPM_RETURNCMD,
						x,
						y,
						0,
						m_hWnd,
						NULL);
				}
				
				/*if(uCommand > 0)
				{
					CMINVOKECOMMANDINFO  cmi;
					
					ZeroMemory(&cmi, sizeof(cmi));
					cmi.cbSize = sizeof(cmi);
					cmi.hwnd = m_hwndParent;
					cmi.lpVerb = (LPCSTR)MAKEINTRESOURCE(uCommand - MENU_OFFSET);
					
					pContextMenu->InvokeCommand(&cmi);
				}*/
				if(uCommand > 0)
				{
					//some commands need to be handled by the view itself
					switch(uCommand)
					{
					case IDM_VIEW_LARGE:
						//OnViewLarge();
						break;
						
					case IDM_VIEW_SMALL:
						//OnViewSmall();
						break;
						
					case IDM_VIEW_LIST:
						//OnViewList();
						break;
						
					case IDM_VIEW_DETAILS:
						//OnViewDetails();
						break;
						
					default:
						{
							CMINVOKECOMMANDINFO  cmi = {0};
							
							//ZeroMemory(&cmi, sizeof(cmi));
							cmi.cbSize = sizeof(cmi);
							cmi.hwnd = m_hwndParent;
							cmi.lpVerb = (LPCSTR)MAKEINTRESOURCE(uCommand - MENU_OFFSET);
							
							pContextMenu->InvokeCommand(&cmi);
						}
						break;
					}
				}

				
				DestroyMenu(hMenu);
			}

			pContextMenu->Release();
		}
		
		m_pMalloc->Free(aSelectedItems);
	}
}

LRESULT CShellView::OnInitMenuPopup(HMENU hMenu)
{
	::EnableMenuItem(hMenu, IDM_VIEW_IDW, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	::CheckMenuItem(hMenu, IDM_VIEW_IDW, MF_BYCOMMAND | MF_UNCHECKED);
	
	return 0;
}

LRESULT CShellView::OnCommand(DWORD dwCmdID, DWORD dwCmd, HWND hwndCmd)
{
	switch(dwCmdID)
	{
	case IDM_CREATE_FOLDER:
		OnCreateNewFolder();
		break;

	case IDM_DELETE:
	case IDM_DELETE_FOLDER:
		OnDelete();
		break;

	case IDM_PROPERTIES:
		OnShowProperties();
		break;

	case IDM_RENAMEFOLDER:
		OnRename();
		break;

	case IDM_REFRESH:
		Refresh();
		break;

	case IDC_LARGE_ICONS:
		OnSetViewStyle( LVS_ICON );
		break;
	case IDC_SMALL_ICONS:
		OnSetViewStyle( LVS_SMALLICON );
		break;
	case IDC_LIST:
		OnSetViewStyle( LVS_LIST );
		break;
	case IDC_DETAILS:
		OnSetViewStyle( LVS_REPORT );
		break;
	case IDC_DETAILS2:
		OnSetViewStyle( LVS_REPORT2 );
		break;
	case IDC_THUMBNAILS:
		::MessageBeep( MB_OK );
		break;

	default:
		break;
	}
	
	return 0;
}

LRESULT CShellView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	DWORD dwCmdID	= GET_WM_COMMAND_ID(wParam, lParam);
	DWORD dwCmd		= GET_WM_COMMAND_CMD(wParam, lParam);
	HWND hwndCmd	= GET_WM_COMMAND_HWND(wParam, lParam);

//	_RPTF1(_CRT_WARN, "OnCommand (%d)\n", dwCmdID);
	switch(dwCmdID)
	{
	case IDM_VIEW_IDW:
		{
			CToolBar::OnToolbarDropdown(
				m_pListView,
				m_pShellBrowser,
				m_hwndParent,
				m_hWnd,
				lParam);
		}
		break;
	default:
		return OnCommand(dwCmdID, dwCmd, hwndCmd);
	}
	
	return 0;
}

void CShellView::OnRename()
{
	if(m_pListView && (1 == m_pListView->GetSelectedCount()))
	{
		LVITEM lvItem;
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = -1;
		lvItem.iSubItem = 0;

		lvItem.iItem = m_pListView->GetNextItem(lvItem.iItem, LVNI_SELECTED);

		m_pListView->SetFocus();
		m_pListView->EditLabel( lvItem.iItem );
	}
}

void CShellView::OnCreateNewFolder()
{
	m_pSFParent->CreateNewFolder(NULL);
}

void CShellView::OnShowProperties()
{
	UINT     i;
	LVITEM   lvItem;
	
	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = -1;
	lvItem.iSubItem = 0;

	if(NULL == m_pListView || (0 == m_pListView->GetSelectedCount()))
	{
		m_pSFParent->ShowProperties( NULL );
	}
	else
	{
		for(i = 0; -1 != (lvItem.iItem = m_pListView->GetNextItem(lvItem.iItem, LVNI_SELECTED)); i++)
		{
			if(m_pListView->GetItem(&lvItem))
			{
				LPITEMIDLIST pidl = (LPITEMIDLIST)lvItem.lParam;
				m_pSFParent->ShowProperties( pidl );
			}
		}
	}
}

void CShellView::OnDelete()
{
	if(NULL == m_pListView)
	{
		m_pSFParent->DeleteFolder( NULL );
		return;
	}

	UINT     i;
	LVITEM   lvItem;
	
	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = -1;
	lvItem.iSubItem = 0;

	LPITEMIDLIST   *pPidls;

	int nCount = m_pListView->GetSelectedCount() + 1;

	if(nCount > 1)
	{
		pPidls = (LPITEMIDLIST*)m_pMalloc->Alloc(nCount * sizeof(LPITEMIDLIST));

		if(pPidls)
		{
			//set all of the entries to NULL
			ZeroMemory(pPidls, nCount * sizeof(LPITEMIDLIST));

			for(i = 0; -1 != (lvItem.iItem = m_pListView->GetNextItem(lvItem.iItem, LVNI_SELECTED)); i++)
			{
				if(m_pListView->GetItem(&lvItem))
				{
					pPidls[i] = g_pPidlMgr->Copy((LPITEMIDLIST)lvItem.lParam);
				}
			}

			m_pSFParent->Delete( pPidls );

			for(i = 0; pPidls[i]; i++)
				g_pPidlMgr->Delete(pPidls[i]);
			m_pMalloc->Free(pPidls);
		}
	}
	else if( !m_pSFParent->IsRoot() )
	{
		m_pSFParent->DeleteFolder( NULL );
	}
}

void CShellView::OnEndTrack()
{
	g_nColumn1 = m_pListView->GetColumnWidth(0);
	g_nColumn2 = m_pListView->GetColumnWidth(1);
}

int CShellView::OnGetDispInfo(LPNMHDR lpnmh)
{
	LV_DISPINFO    *lpdi = (LV_DISPINFO *)lpnmh;
	LPITEMIDLIST   pidl = (LPITEMIDLIST)lpdi->item.lParam;

	if(lpdi->item.iSubItem)
	{
		if(lpdi->item.mask & LVIF_TEXT)
		{
			if( CPidlManager::IsFile(pidl) )
			{
				LPPIDLDATA  pData = CPidlManager::GetDataPointer(pidl);

				lstrcpy(lpdi->item.pszText, pData->fileData.szPath);
				
				if(!*lpdi->item.pszText)
					LoadString(
						g_hInst,
						IDS_NODATA,
						lpdi->item.pszText,
						lpdi->item.cchTextMax);
			}
			else
			{
				LPPIDLDATA pData = CPidlManager::GetDataPointer(pidl);
				lstrcpyn(lpdi->item.pszText, pData->folderData.szQuery, lpdi->item.cchTextMax);
				if(!*lpdi->item.pszText)
					LoadString(
						g_hInst,
						IDS_FOLDER,
						lpdi->item.pszText,
						lpdi->item.cchTextMax);
			}
		}
	}
	//the item text is being requested
	else
	{
		//is the text being requested?
		if(lpdi->item.mask & LVIF_TEXT)
		{
			STRRET   str;
			
			if(SUCCEEDED(m_pSFParent->GetDisplayNameOf(pidl, SHGDN_NORMAL | SHGDN_INFOLDER, &str)))
			{
				if(STRRET_WSTR == str.uType)
				{
					CString::WideCharToLocal(lpdi->item.pszText, str.pOleStr, lpdi->item.cchTextMax);
					
					//delete the string buffer
					m_pMalloc->Free(str.pOleStr);
				}
			}
		}
		
		//is the image being requested?
		if(lpdi->item.mask & LVIF_IMAGE)
		{
			IExtractIcon   *pei;
			
			if(SUCCEEDED(m_pSFParent->GetUIObjectOf(m_hWnd, 1, (LPCITEMIDLIST*)&pidl, IID_IExtractIcon, NULL, (LPVOID*)&pei)))
			{
				UINT  uFlags;
				
				//GetIconLoaction will give us the index into our image list
				pei->GetIconLocation(GIL_FORSHELL, NULL, 0, &lpdi->item.iImage, &uFlags);
				
				pei->Release();
			}
		}
	}
	return 0;
}

BOOL CShellView::InitList(void)
{
	if(!m_pListView)
		return FALSE;
	
	m_pListView->Clear();

	TCHAR szString[MAX_PATH];

	LoadString(g_hInst, IDS_COLUMN1, szString, sizeof(szString));
	m_pListView->InsertColumn(szString, g_nColumn1);

	LoadString(g_hInst, IDS_COLUMN2, szString, sizeof(szString));
	m_pListView->InsertColumn(szString, g_nColumn2);

//	m_pListView->SetImageList(g_pIcons->GetListSmall(), LVSIL_SMALL);
//	m_pListView->SetImageList(g_pIcons->GetListLarge(), LVSIL_NORMAL);
	m_pListView->SetImageList(m_pSFParent->GetListSmall(), LVSIL_SMALL);
	m_pListView->SetImageList(m_pSFParent->GetListLarge(), LVSIL_NORMAL);

	return TRUE;
}

void CShellView::FillList(void)
{
	if(NULL == m_pListView)
		return;

	LPENUMIDLIST   pEnumIDList;
	
	if(SUCCEEDED(m_pSFParent->EnumObjects(m_hWnd, SHCONTF_NONFOLDERS | SHCONTF_FOLDERS, &pEnumIDList)))
	{
		LPITEMIDLIST   pidl;
		DWORD          dwFetched;
		
		m_pListView->SetRedraw(FALSE);
		
		while((S_OK == pEnumIDList->Next(1, &pidl, &dwFetched)) && dwFetched)
		{
			InsertItem(pidl);
		}
		
		m_pListView->SortItems(SortFunc, 0);
		
		m_pListView->SetRedraw(TRUE);

		m_pListView->Invalidate();
		
		pEnumIDList->Release();

		if(m_iLastSelectedItem > -1)
		{
			m_pListView->SelectItem(m_iLastSelectedItem);
			m_iLastSelectedItem = -1;
		}
	}

	if(m_pListView)
	{
		LONG style = m_pSFParent->GetListViewStyle();
		m_pListView->SetStyle( style );
	}

	CStatusBar::Fill(m_pShellBrowser, m_pSFParent);

#ifdef USE_ADSENSE
	m_pWebBrowser->Navigate("http://www.google.com");
#endif
}

void CShellView::InitMenu(UINT uState)
{
	// Override
}

int CShellView::FindItemPidl(LPCITEMIDLIST pidl)
{
	LVITEM lvItem = {0};
	
	//ZeroMemory(&lvItem, sizeof(lvItem));
	lvItem.mask = LVIF_PARAM;
	
	for(lvItem.iItem = 0; m_pListView->GetItem(&lvItem); lvItem.iItem++)
	{
		LPITEMIDLIST   pidlFound = (LPITEMIDLIST)lvItem.lParam;
		HRESULT        hr = m_pSFParent->CompareIDs(0, pidl, pidlFound);
		if(SUCCEEDED(hr) && 0 == HRESULT_CODE(hr))
		{
			return lvItem.iItem;
		}
	}
	
	return -1;
}

VOID CShellView::UpdateData(LPCITEMIDLIST pidl)
{
	int i = FindItemPidl(pidl);
	
	if(-1 != i)
	{
		m_pListView->Update(i);
	}
}

void CShellView::AddNewFolder(LPCITEMIDLIST pidl)
{
	LPITEMIDLIST pidlLocal;
	LPITEMIDLIST pidlLast = CPidlManager::GetLastItem(pidl);
	pidlLocal = g_pPidlMgr->Copy(pidlLast);
	int index = InsertItem(pidlLocal);
	if(index >= 0)
	{
		m_pListView->SetFocus();
		m_pListView->EditLabel(index);
		m_iLastSelectedItem = index;
	}
}

int CShellView::InsertItem(LPCITEMIDLIST pidl)
{
	if(m_pCommDlgBrowser)
	{
		HRESULT hr;
		hr = m_pCommDlgBrowser->IncludeObject((IShellView*)this, pidl);
		if(S_OK != hr)
		{
			TRACE_PIDL_DATA("IncludeObject (%s) returned S_FALSE\n", pidl);
		}
	}

	if(NULL != m_pListView)
		return m_pListView->InsertItem( (LPARAM)g_pPidlMgr->Copy(pidl) );

	return -1;
}

void CShellView::OnDefaultAction()
{
	if(m_pCommDlgBrowser)
	{
		if(S_OK == m_pCommDlgBrowser->OnDefaultCommand(this))
			return;
	}
	DoContextMenu(0, 0, TRUE);
}

void CShellView::OnItemChanged(LPNMLISTVIEW pnmlv)
{
	if(LVIF_STATE & pnmlv->uChanged)
	{
		if((LVIS_SELECTED & pnmlv->uOldState) != (LVIS_SELECTED & pnmlv->uNewState))
		{
			StateChange(CDBOSC_SELCHANGE);
		}
	}
}

HRESULT CShellView::StateChange(ULONG uChange)
{
	if(m_pCommDlgBrowser)
	{
		return m_pCommDlgBrowser->OnStateChange((IShellView*)this, uChange);
	}
	return E_NOTIMPL;
}

void CShellView::OnSetViewStyle(LONG style)
{
	if(m_pListView)
	{
		m_pListView->SetStyle(style);
		m_pSFParent->SaveListViewStyle(style);
	}
}
