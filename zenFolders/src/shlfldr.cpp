
#include <crtdbg.h>
#include <stdio.h>

#include "zenfolders.h"
#include "shlfldr.h"
#include "shlview.h"
#include "infotip.h"
#include "extricon.h"
#include "enumidl.h"
#include "contmenu.h"
#include "guid.h"
#include "dataobj.h"
#include "resource.h"
#include "viewlist.h"
#include "droptrgt.h"
#include "cfgxml.h"
#include "resource.h"

#include "util/string.h"
#include "util/settings.h"
#include "util/utilities.h"


extern HINSTANCE	g_hInst;
extern LPVIEWSLIST	g_pViewList;
extern LPCONFIGXML	g_pConfigXML;
extern LPPIDLMGR	g_pPidlMgr;


CShellFolder::CShellFolder()
{
	if( !Init() )
	{
		delete this;
		return;
	}
}

CShellFolder::CShellFolder(CShellFolder *pParent, LPCITEMIDLIST pidl)
{
	if(!pParent || !pidl)
	{
		delete this;
		return;
	}

	if( !Init() )
	{
		delete this;
		return;
	}

	m_pSFParent = pParent;
	m_pSFParent->AddRef();

	m_pidlRel = pidl;

//	TRACE_PIDL_PATH("CShellFolder(%s)\n", &m_pidlRel);
}

CShellFolder::~CShellFolder()
{
	if(m_pSFParent)
		m_pSFParent->Release();
	
	if(m_pMalloc)
		m_pMalloc->Release();

	if(m_pIcons)
		delete m_pIcons;

	gRelease(CSHELLFOLDER);
}

bool CShellFolder::Init()
{
	m_pSFParent		= NULL;
	m_iFileCount	= 0;

	if(FAILED(::SHGetMalloc(&m_pMalloc)))
	{
		_RPTF0(_CRT_ERROR, "SHGetMalloc failed\n");
		return false;
	}

	m_pIcons = new CIconList(g_hInst);

	m_ObjRefCount = 1;

	gAddRef(CSHELLFOLDER);

	return true;
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

STDMETHODIMP CShellFolder::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;
	
	//IUnknown
	if(IsEqualIID(riid, IID_IUnknown))
		*ppReturn = this;
	
	//IPersistFolder
	else if(IsEqualIID(riid, IID_IPersistFolder))
		*ppReturn = (IPersistFolder*)this;
	
	//IShellFolder
	else if(IsEqualIID(riid, IID_IShellFolder))
		*ppReturn = (IShellFolder*)this;
	
	if(*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CShellFolder::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CShellFolder::Release()
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
// IPersist Implementation
//

/**************************************************************************
IPersist::GetClassID
	Retrieves the class identifier (CLSID) of an object. The CLSID is a
	unique value that identifies the code that can manipulate the
	persistent data.

Parameter
	pClassID [out]	Points to the location of the CLSID on return.
					The CLSID is a globally unique identifier (GUID)
					that uniquely represents an object class that defines
					the code that can manipulate the object's data.

Return Values
	S_OK	The CLSID was successfully retrieved.
	E_FAIL	The CLSID could not be retrieved.
**************************************************************************/
STDMETHODIMP CShellFolder::GetClassID(LPCLSID lpClassID)
{
	*lpClassID = CLSID_ShellFolderNameSpace;
	
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
// IPersistFolder Implementation
//

/**************************************************************************
IPersistFolder::Initialize
	This method instructs a shell folder object to initialize itself based
	on the information passed.

Parameters
	pidl	Address of the ITEMIDLIST structure that specifies the absolute
			location of the folder.

Return Value
	Returns NOERROR if successful, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellFolder::Initialize(LPCITEMIDLIST pidl)
{
//	TRACE_PIDL_PATH("CShellFolder::Initialize(%s)\n", pidl);

	m_pidlFQ = pidl;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
// IShellFolder Implementation
//

/**************************************************************************
IShellFolder::BindToObject
	This method retrieves an IShellFolder object for a subfolder.

Parameters
	pidl [in]		Pointer to an ITEMIDLIST structure (PIDL)
					that identifies the subfolder.
	pbc [in]		Optional address of an IBindCtx interface
					on a bind context object to be used during
					this operation. If this parameter is not used,
					set it to NULL. Because support for pbc is
					optional for folder object implementations,
					some folders may not support the use of bind contexts.
	riid [in]		Identifier of the interface to return.
	ppvOut [out]	Address that receives the interface pointer.
					If an error occurs, a NULL pointer is returned
					in this address.

Return Values
	Returns NOERROR if successful, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellFolder::BindToObject(LPCITEMIDLIST pidl, 
										LPBC pbcReserved, 
										REFIID riid, 
										LPVOID *ppvOut)
{
//	TRACE_PIDL_PATH("CShellFolder::BindToObject(%s)\n", pidl);

	*ppvOut = NULL;
/*
	if( !CPidlManager::IsFolder(pidl) )
	{
		IShellFolder *psfParent;
		LPITEMIDLIST pidlRelative;
		LPCITEMIDLIST fsPidl = g_pPidlMgr->GetFSPidl(pidl);
		HRESULT hr = ::SHBindToParent(
			fsPidl,
			IID_IShellFolder,
			(void **)&psfParent,
			(LPCITEMIDLIST*)&pidlRelative);
		if( SUCCEEDED(hr) )
		{
			hr = psfParent->BindToObject(pidlRelative, pbcReserved, riid, ppvOut);
		}
		return hr;
	}
*/
	CShellFolder *pShellFolder = new CShellFolder(this, pidl);
	if(!pShellFolder)
		return E_OUTOFMEMORY;

	LPITEMIDLIST pidlFQ = CreateFQPidl(pidl);
	pShellFolder->Initialize(pidlFQ);
	g_pPidlMgr->Delete(pidlFQ);

	HRESULT hr = pShellFolder->QueryInterface(riid, ppvOut);
	
	pShellFolder->Release();
	
	return hr;
}

/**************************************************************************
IShellFolder::BindToStorage
	Requests a pointer to an object's storage interface.

Parameters
	pidl [in]		Address of an ITEMIDLIST structure that identifies
					the subfolder relative to its parent folder.
					The structure must contain exactly one SHITEMID
					structure followed by a terminating zero.
	pbc [in]		Optional address of an IBindCtx interface on a bind
					context object to be used during this operation.
					If this parameter is not used, set it to NULL.
					Because support for pbc is optional for folder object
					implementations, some folders may not support the use
					of bind contexts.
	riid [in]		IID of the requested storage interface. To retrieve
					an IStream, IStorage, or IPropertySetStorage interface
					pointer, set riid to IID_IStream, IID_IStorage, or
					IID_IPropertySetStorage, respectively.
	ppvOut [out]	Address that receives the interface pointer specified
					by riid. If an error occurs, a NULL pointer is returned
					in this address.

Return Value
    Returns S_OK if successful, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellFolder::BindToStorage(LPCITEMIDLIST pidl, 
										 LPBC pbcReserved, 
										 REFIID riid, 
										 LPVOID *ppvOut)
{
	TRACE_PIDL_PATH("CShellFolder::BindToStorage(%s)\n", pidl);

	*ppvOut = NULL;

	return E_NOTIMPL;
}

/**************************************************************************
IShellFolder::CompareIDs
	This method determines the relative order of two file objects
	or folders, given their item identifier lists.

Parameters
	lParam [in]	Value that specifies how the comparison should be performed.
	pidl1 [in]	Pointer to the first item's ITEMIDLIST structure.
				It is relative to the folder. This ITEMIDLIST structure
				can contain more than one element; therefore, the entire
				structure must be compared, not just the first element.
	pidl2 [in]	Pointer to the second item's ITEMIDLIST structure.
				It will be relative to the folder. This ITEMIDLIST
				structure can contain more than one element; therefore,
				the entire structure must be compared, not just the first element.

Return Values
	If this method is successful, the CODE field of the HRESULT
	contains one of the values in the following table.
	Negative	A negative return value indicates that the first item
				should precede the second (pidl1 < pidl2).
	Positive	A positive return value indicates that the first item
				should follow the second (pidl1 > pidl2).
	Zero		A return value of zero indicates that the two items are
				the same (pidl1 = pidl2).
**************************************************************************/
STDMETHODIMP CShellFolder::CompareIDs(LPARAM lParam, 
									  LPCITEMIDLIST pidl1, 
									  LPCITEMIDLIST pidl2)
{
	return CPidlManager::CompareIDs(pidl1, pidl2);
}

/**************************************************************************
IShellFolder::CreateViewObject
	This method requests an object that can be used to obtain
	information from or interact with a folder object.

Parameters
	hwndOwner [in]	Handle to the owner window. If you have
					implemented a custom folder view object,
					your folder view window should be created
					as a child of hwndOwner.
	riid [in]		Identifier of the requested interface.
	ppvOut [out]	Address of a pointer to the requested interface.

Return Values
	Returns NOERROR if successful, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellFolder::CreateViewObject(HWND hwndOwner, 
											REFIID riid, 
											LPVOID *ppvOut)
{
	HRESULT hr = E_NOINTERFACE;

	if(IsEqualIID(riid, IID_IShellView))
	{
		*ppvOut = NULL;
		CShellView *pShellView;
		LPITEMIDLIST pidl = m_pidlRel.GetRelative();
		pShellView = new CShellView(this, pidl);
		if(!pShellView)
			return E_OUTOFMEMORY;
		
		hr = pShellView->QueryInterface(riid, ppvOut);
		
		pShellView->Release();

		*ppvOut = pShellView;
	}
	else if(IsEqualIID(riid, IID_IDropTarget))
	{
		*ppvOut = new CDropTarget(this);
		
		if( *ppvOut )
			return S_OK;
	}
	
	return hr;
}

/**************************************************************************
IShellFolder::EnumObjects
	This method allows a client to determine the contents of a
	folder by creating an item identifier enumeration object and
	returning its IEnumIDList interface. The methods supported by
	that interface can be used to enumerate the folder's contents.

Parameters
	hwndOwner [in]		Window handle that the enumeration object
						should use as that parent window to take
						user input, if user input is required.
						An example would be a dialog box that asks
						for a password or prompts the user to insert
						a CD or floppy disk. If hwndOwner is set to NULL,
						the enumerator should not post any messages.
						If user input is required, it should silently fail.
	grfFlags [in]		Flags indicating which items to include in the
						enumeration. For a list of possible values, see
						the SHCONTF enumerated type.
	ppenumIDList [out]	Address that receives a pointer to the IEnumIDList
						interface of the enumeration object created by this
						method. If an error occurs, ppenumIDList is set to NULL.

Return Values
	Returns NOERROR if successful, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellFolder::EnumObjects(HWND hwndOwner, 
									   DWORD dwFlags, 
									   LPENUMIDLIST *ppEnumIDList)
{
	HRESULT  hr;
	
	*ppEnumIDList = CreateList(dwFlags, &hr);
	if(NULL == *ppEnumIDList)
		return hr;
	
	return S_OK;
}

/**************************************************************************
IShellFolder::GetAttributesOf
	This method retrieves the attributes of one or more file
	objects or subfolders.

Parameters
	cidl [in]			Number of file objects from which to
						retrieve attributes.
	apidl [in]			Address of an array of pointers to
						ITEMIDLIST structures, each of which uniquely
						identifies a file object relative to the parent
						folder. Each ITEMIDLIST structure must contain
						exactly one SHITEMID structure followed by a
						terminating zero.
	rgfInOut [in, out]	Address of a single ULONG value that, on entry,
						contains the attributes that the caller is requesting.
						On exit, this value contains the requested attributes
						that are common to all of the specified objects.
						This is the address of a single ULONG value, not an
						array of ULONG values. See Remarks for possible values.

Return Values
	Returns NOERROR if successful, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellFolder::GetAttributesOf(UINT uCount, 
										   LPCITEMIDLIST aPidls[], 
										   LPDWORD pdwAttribs)
{
	UINT  i;
	
	if(0 == uCount)
		*pdwAttribs = SFGAO_FOLDER | 
                  SFGAO_HASSUBFOLDER | 
                  SFGAO_BROWSABLE | 
                  SFGAO_FILESYSANCESTOR |
                  SFGAO_FILESYSTEM |
				  SFGAO_HASPROPSHEET,
                  0;
	
	for(i=0; i<uCount; i++)
	{
		DWORD dwAttribs = 0;
		
		//flags common to all items
		dwAttribs |= SFGAO_CANRENAME | 
					  SFGAO_CANDELETE | 
					  SFGAO_CANCOPY | 
					  SFGAO_CANMOVE,
					  0;
		
		//is this item a folder?

		dwAttribs |= SFGAO_FOLDER | 
					 SFGAO_BROWSABLE | 
					 SFGAO_CANLINK |
					 SFGAO_FILESYSANCESTOR |
					 SFGAO_FILESYSTEM |
					 SFGAO_DROPTARGET,
					 0;
		
		CPidl pidlFQ = CreateFQPidl(aPidls[i]);
		if( CPidlManager::HasSubFolders(pidlFQ) )
			dwAttribs |= SFGAO_HASSUBFOLDER;

		*pdwAttribs &= dwAttribs;
	}

	return S_OK;
}

/**************************************************************************
IShellFolder::GetUIObjectOf
	This method retrieves a COM interface that can be used to carry
	out actions on the specified file objects or folders.

Parameters
	hwndOwner [in]	Handle to the owner window that the client should
					specify if it displays a dialog box or message box.
	cidl [in]		Number of file objects or subfolders specified in the
					apidl parameter.
	apidl [in]		Address of an array of pointers to ITEMIDLIST structures,
					each of which uniquely identifies a file object or
					subfolder relative to the parent folder. Each item
					identifier list must contain exactly one SHITEMID
					structure followed by a terminating zero.
	riid [in]		Identifier of the COM interface object to return.
					This can be any valid interface identifier that can be
					created for an item. The most common identifiers used
					by the shell are listed in Remarks.
	rgfReserved		Reserved. 
	ppv [out]		Pointer to the requested interface. If an error occurs,
					a NULL pointer is returned in this address.

Return Values
	Returns NOERROR if successful, E_NOINTERFACE if the interface is not
	supported, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellFolder::GetUIObjectOf(HWND hwndOwner, 
										 UINT uCount, 
										 LPCITEMIDLIST *pPidl, 
										 REFIID riid, 
										 LPUINT puReserved, 
										 LPVOID *ppvReturn)
{
	*ppvReturn = NULL;

	//IID_IContextMenu
	if(IsEqualIID(riid, IID_IContextMenu))
	{
		*ppvReturn = new CContextMenu(this, pPidl, uCount);
		
		if( *ppvReturn )
			return S_OK;
	}
	
	//IID_IDataObject
	if(IsEqualIID(riid, IID_IDataObject))
	{
		*ppvReturn = new CDataObject(this, pPidl, uCount);
		
		if( *ppvReturn )
			return S_OK;
	}

	if(uCount != 1)
		return E_FAIL;
	
	//IID_IExtractIcon
	if(IsEqualIID(riid, IID_IExtractIcon))
	{
		CPidl pidl = m_pidlRel + pPidl[0];
		*ppvReturn = new CExtractIcon( this, pidl );
		
		if( *ppvReturn )
			return S_OK;
		
		return E_OUTOFMEMORY;
	}
	else if(IsEqualIID(riid, IID_IDropTarget))
	{
		CShellFolder *psfTemp = NULL;
		
		BindToObject(pPidl[0], NULL, IID_IShellFolder, (LPVOID*)&psfTemp);
		
		if(psfTemp)
		{
			*ppvReturn = new CDropTarget(psfTemp);
			
			psfTemp->Release();
			
			if( *ppvReturn )
				return S_OK;
		}
	}

//#if (_WIN32_IE >= 0x0400)
	if(IsEqualIID(riid, IID_IQueryInfo))
	{
		CPidl pidl = m_pidlRel + pPidl[0];

		*ppvReturn = new CQueryInfo( pidl );
		
		if( *ppvReturn )
			return S_OK;
		
		return E_OUTOFMEMORY;
	}
//#endif   //#if (_WIN32_IE >= 0x0400)
	
	return E_NOINTERFACE;
}

#define GET_SHGDN_FOR(dwFlags)         ((DWORD)dwFlags & (DWORD)0x0000FF00)
#define GET_SHGDN_RELATION(dwFlags)    ((DWORD)dwFlags & (DWORD)0x000000FF)

/**************************************************************************
IShellFolder::GetDisplayNameOf
	This method retrieves the display name for the specified file
	object or subfolder.

Parameters
	pidl [in]		Pointer to an ITEMIDLIST structure (PIDL)
					that uniquely identifies the file object or
					subfolder relative to the parent folder.
	uFlags [in]		Flags used to request the type of display name
					to return. For a list of possible values, see
					the SHGNO enumerated type.
	lpName [out]	Address of a STRRET structure in which to return
					the display name. The type of name returned in
					this structure can be the requested type, but
					the shell folder might return a different type.

Return Values
	Returns NOERROR if successful, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, 
											DWORD dwFlags, 
											LPSTRRET lpName)
{
	int cchOleStr;
	TCHAR szText[MAX_PATH] = {0};
	LPITEMIDLIST pidlRel = CPidlManager::GetLastItem(pidl);

	if (NULL==pidl || pidl->mkid.cb == 0)//root folder
	{
		CPidlManager::GetItemName(pidlRel, szText, ARRAYSIZE(szText));
	}
	else
	{
//		SHGNO a = (SHGNO)GET_SHGDN_FOR(dwFlags);
//		SHGNO b = (SHGNO)GET_SHGDN_RELATION(dwFlags);

		switch(GET_SHGDN_FOR(dwFlags))
		{
		case SHGDN_FOREDITING:
		case SHGDN_FORADDRESSBAR:
			CPidlManager::GetItemName(pidlRel, szText, ARRAYSIZE(szText));
			break;
//		case SHGDN_FORPARSING:
//			break;
		default:
			switch(GET_SHGDN_RELATION(dwFlags))
			{
			case SHGDN_INFOLDER:
				CPidlManager::GetItemName(pidlRel, szText, ARRAYSIZE(szText));
				break;
			case SHGDN_NORMAL:
				{
					CPidl cpidl(pidl);
					if( cpidl.IsFile() )
					{
						cpidl.GetFSPath(szText, ARRAYSIZE(szText));
					}
					else
					{
						LPITEMIDLIST pidlFQ = CreateFQPidl(pidl);
						//::SHGetPathFromIDList(pidlFQ, szText);
						CPidlManager::GetPidlPath(pidlFQ, szText, ARRAYSIZE(szText));
						g_pPidlMgr->Delete(pidlFQ);
					}
				}
				break;
			default:
				return E_INVALIDARG;
			}
		}
	}

	//get the number of characters required
	cchOleStr = lstrlen(szText) + 1;

//	_RPTF1(_CRT_WARN, "CShellFolder::GetDisplayNameOf(%s)\n", szText);
	
	//allocate the wide character string
	lpName->pOleStr = (LPWSTR)m_pMalloc->Alloc(cchOleStr * sizeof(WCHAR));
	if(!lpName->pOleStr)
		return E_OUTOFMEMORY;
	
	lpName->uType = STRRET_WSTR;
	
	CString::LocalToWideChar(lpName->pOleStr, szText, cchOleStr);

	return S_OK;
}

/**************************************************************************
IShellFolder::ParseDisplayName
	This method translates the display name of a file object or folder
	into an item identifier list.

Parameters
	hwnd [in]				Optional window handle. The client should
							provide a window handle if it displays a dialog
							or message box. Otherwise, set hwnd to NULL.
	pbc [in]				Optional bind context that controls the parsing
							operation. This parameter is typically set to NULL.
	pwszDisplayName [in]	Null-terminated UNICODE string with the display name.
							Because each shell folder defines its own parsing
							syntax, the form this string can take may vary.
							The desktop folder, for instance, accepts paths such
							as "c:\My Docs\My File.txt". It also accepts references
							to items in the namespace that have a globally unique
							identifier (GUID) associated with them using the
							"::{GUID}" syntax. For example, to retrieve a fully
							qualified identifier list for the control panel from
							the desktop folder, you can use
							"{CLSID for My Computer}\::{CLSID for the Control Panel}".
	pchEaten [out]			Pointer to a ULONG value that receives the number of
							characters of the display name that was parsed. If your
							application does not need this information, set pchEaten
							to NULL, and no value will be returned.
	ppidl [out]				Pointer to an ITEMIDLIST pointer that receives the item
							identifier list for the object. The returned item
							identifier list specifies the item relative to the
							parsing folder. If the object associated with
							pwszDisplayName is within the parsing folder, the
							returned item identifier list contains only one
							SHITEMID structure. If the object is in a subfolder
							of the parsing folder, the returned item identifier list
							contains multiple SHITEMID structures. If an error occurs,
							NULL is returned in this address.
	pdwAttributes [in, out]	Optional parameter that can be used to query for file
							attributes. If not used, it should be set to NULL.
							To query for one or more attributes, initialize the
							pdwAttributes with the flags that represent the
							attributes of interest. On return, those attributes
							that are true and were requested are set. See Remarks
							for possible values.

Return Values
	Returns NOERROR if successful, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellFolder::ParseDisplayName(HWND hwndOwner, 
											LPBC pbcReserved, 
											LPOLESTR lpDisplayName, 
											LPDWORD pdwEaten, 
											LPITEMIDLIST *pPidlNew, 
											LPDWORD pdwAttributes)
{
	TCHAR szName[MAX_PATH] = {0};
	MSXML2::IXMLDOMNodePtr ptrNode;
	MSXML2::IXMLDOMNodePtr ptrChild;

	CString::WideCharToLocal(szName, lpDisplayName, ARRAYSIZE(szName));

//	_RPTF1(_CRT_WARN, "CShellFolder::ParseDisplayName(%s)\n", szName);
//	TRACE_PIDL_PATH("CShellFolder::ParseDisplayName(m_pidlFQ = %s)\n", &m_pidlFQ);

	ptrNode = m_pidlFQ.GetNode();
	if(NULL == ptrNode)
		return E_FAIL;

	//_RPTF1(_CRT_WARN, "CShellFolder::ParseDisplayName(%d)\n", (pdwAttributes && SFGAO_FOLDER));

	ptrChild = g_pConfigXML->GetSubfolder(ptrNode, szName);
	if(ptrChild == NULL)
	{
		// File
		//TRACE_PIDL_PATH("CShellFolder::ParseDisplayName(m_pidlFQ = %s)\n", &m_pidlFQ);
		_RPTF1(_CRT_WARN, "Get the file '%s'\n", szName);
		//HRESULT hr;
		//CEnumIDList *pEnumIdList = CreateList(SHCONTF_NONFOLDERS, &hr);
		//delete pEnumIdList;
		return E_FAIL;
	}

	*pPidlNew = g_pPidlMgr->CreateFolder(ptrChild);
//	TRACE_PIDL_PATH("CShellFolder::ParseDisplayName(pPidlNew = %s)\n", *pPidlNew);

	return NOERROR;
}

/**************************************************************************
IShellFolder::SetNameOf
	This method sets the display name of a file object or subfolder,
	changing the item identifier in the process.

Parameters
	hwndOwner [in]	Handle to the owner window of any dialog or message
					boxes that the client displays.
	pidl [in]		Pointer to an ITEMIDLIST structure that uniquely
					identifies the file object or subfolder relative to
					the parent folder. The structure must contain exactly
					one SHITEMID structure followed by a terminating zero.
	lpszName [in]	Pointer to a null-terminated string that specifies the
					new display name.
	uFlags [in]		Flags indicating the type of name specified by the
					lpszName parameter. For a list of possible values, see
					the description of the SHGNO enumerated type.
	ppidlOut [out]	Address of a pointer to an ITEMIDLIST structure that
					receives the new ITEMIDLIST.

Return Values
	Returns NOERROR if successful, or an error value otherwise.
**************************************************************************/
STDMETHODIMP CShellFolder::SetNameOf(HWND hwndOwner, 
									 LPCITEMIDLIST pidl, 
									 LPCOLESTR lpName, 
									 DWORD dw, 
									 LPITEMIDLIST *pPidlOut)
{
	TCHAR szName[MAX_PATH] = {0};

	CString::WideCharToLocal(szName, (LPOLESTR)lpName, ARRAYSIZE(szName));

	if(0 == lstrlen(szName))
		return E_FAIL;

	MSXML2::IXMLDOMNodePtr ptrNode = m_pidlFQ.GetNode();
	if(NULL != g_pConfigXML->GetSubfolder(ptrNode, szName))
		return E_FAIL;

	CPidl pidlNew(pidl);

	if( pidlNew.IsFolder() )
	{
		LPPIDLDATA pData = pidlNew.GetData();
		lstrcpy(pData->szName, szName);

//		if(0 == lstrlen(pData->searchData.szQuery))
//			lstrcpy(pData->searchData.szQuery, szName);

		if(pPidlOut)
		{
			memcpy(*pPidlOut, pidl, sizeof(ITEMIDLIST));
			LPPIDLDATA pDataTmp = CPidlManager::GetDataPointer(*pPidlOut);
			memcpy(pDataTmp, pData, sizeof(PIDLDATA));
		}
		
		LPITEMIDLIST pidlFQOld = CreateFQPidl( pidl );
		LPITEMIDLIST pidlFQNew = CreateFQPidl( pidlNew.GetRelative() );
		if( g_pConfigXML->SaveFolder(pidlFQOld, pidlFQNew) )
		{
			::SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_IDLIST, pidlFQOld, pidlFQNew);
			return NOERROR;
		}
	}

	return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////
//
// Overridable functions
//

LPITEMIDLIST CShellFolder::CreateFQPidl(LPCITEMIDLIST pidl)
{
	return m_pidlFQ + pidl;
}
/*
BOOL CShellFolder::GetFolderPath(LPTSTR lpszOut, DWORD dwOutSize)
{
	CPidlManager::GetPidlPath(m_pidlFQ, lpszOut, dwOutSize);
	return TRUE;
}
*/
LPENUMIDLIST CShellFolder::CreateList(DWORD dwFlags, HRESULT *hr)
{
	CEnumIDList *list = new CEnumIDList(m_pidlFQ, dwFlags, hr);
	m_iFileCount = list->GetFileCount();
	m_iFolderCount = list->GetFolderCount();
	return list;
}

BOOL CShellFolder::IsRoot()
{
	return (NULL == m_pSFParent);
}

void CShellFolder::ShowProperties(LPCITEMIDLIST pidl)
{
	pidl = pidl ? pidl : (IsRoot() ? NULL : m_pidlFQ);
	
	if( NULL == pidl )
		DisplayVersion();
	else if( CPidlManager::IsFolder(pidl) )
		DisplayFolderProperties( pidl );
	else 
		DisplayShellProperties( pidl );
}

void CShellFolder::DisplayFolderProperties(LPCITEMIDLIST pidl)
{
	TRACE_PIDL_PATH("DisplayFolderProperties(%s)\n", pidl);
	TRACE_PIDL_PATH("m_pidlFQ = %s\n", m_pidlFQ);
	TRACE_PIDL_PATH("m_pidlRel = %s\n", m_pidlRel);

	CShellFolder *pParent = m_pSFParent;
	LPCITEMIDLIST tmpPidl = pidl;

	if(NULL == m_pSFParent)
	{
		tmpPidl = CreateFQPidl(pidl);
		pParent = this;
	}

	if((NULL == pParent) || (NULL == tmpPidl))
		return;

	TRACE_PIDL_PATH("CFolderPropertiesDlg(pParent, %s)\n", tmpPidl);

	CFolderPropertiesDlg *pDlg;
	pDlg = new CFolderPropertiesDlg(pParent, tmpPidl);
	if(pDlg)
		pDlg->Show();
}

void CShellFolder::DisplayShellProperties(LPCITEMIDLIST pidl)
{
	LPPIDLDATA pData = CPidlManager::GetDataPointer(pidl);
	SHELLEXECUTEINFO sei = {0};
	//ZeroMemory(&sei, sizeof(sei));
	sei.cbSize	= sizeof(SHELLEXECUTEINFO);
	sei.lpFile	= pData->fileData.szPath;
	sei.nShow	= SW_SHOW;
	sei.fMask	= SEE_MASK_INVOKEIDLIST;
	sei.lpVerb	= "properties";
	::ShellExecuteEx(&sei);
}

BOOL CShellFolder::DeleteFolder(LPCITEMIDLIST pidl, BOOL bVerify)
{
	if((NULL == pidl) && (!IsRoot()))
	{
		//TRACE_PIDL_PATH("CShellFolder::DeleteFolder(%s)\n", &m_pidlRel);
		m_pSFParent->DeleteFolder(m_pidlRel, bVerify);
	}
	else
	{
		CPidl pidlFQ = CreateFQPidl(pidl);
		LPPIDLDATA pData = pidlFQ.GetData();

		if(bVerify)
		{
			TCHAR szQuestion[200] = {0};
			::LoadString(g_hInst, IDS_DELETEFOLDER, szQuestion, ARRAYSIZE(szQuestion));

			TCHAR szCaption[50] = {0};
			::LoadString(g_hInst, IDS_DELETEFOLDER_CAPTION, szCaption, ARRAYSIZE(szCaption));

			TCHAR msg[100] = {0};
			wsprintf(msg, szQuestion, pData->szName);

			int nResult = ::MessageBox( NULL/*hWnd*/, msg, szCaption, MB_YESNO);
			if(IDYES != nResult)
				return FALSE;
		}

		//TRACE_PIDL_PATH("CShellFolder::DeleteFolder(%s)\n", &pidlFQ);
		if( g_pConfigXML->DeleteFolder(pidlFQ) )
		{
			//_RPTF1(_CRT_WARN, "CShellFolder::DeleteFolder(%s)\n", pData->szName);
			::SHChangeNotify(SHCNE_RMDIR, SHCNF_IDLIST, pidlFQ, NULL);
			g_pViewList->Refresh();
		}
	}

	return FALSE;
}

BOOL CShellFolder::DeleteFolderLink(LPCITEMIDLIST pidl, BOOL bVerify)
{
	return DeleteFolder(pidl, bVerify);
}

BOOL CShellFolder::DeleteFile(LPCITEMIDLIST pidl, BOOL bVerify)
{
	if( !CPidlManager::IsFile(pidl) )
	{
		_RPTF0(_CRT_WARN, "Not a file\n");
		return FALSE;
	}

	CPidl cpidl(pidl);
	LPPIDLDATA pData = cpidl.GetData();

	SHFILEOPSTRUCT shfos = {0};

	shfos.wFunc		= FO_DELETE; //FO_COPY FO_DELETE FO_MOVE FO_RENAME 
	shfos.pFrom		= pData->fileData.szPath;
	shfos.fFlags	= FOF_ALLOWUNDO;
	if(!bVerify)
		shfos.fFlags |= FOF_NOCONFIRMATION;

	int result = ::SHFileOperation(&shfos);
	return ( 0 == result );
}

BOOL CShellFolder::DeletePidl(LPCITEMIDLIST pidl, BOOL bVerify)
{
	if( CPidlManager::IsFile(pidl) )
		return DeleteFile( pidl, bVerify );
	else if( CPidlManager::IsFolder(pidl) )
		return DeleteFolder( pidl, bVerify );
	else if( CPidlManager::IsFolderLink(pidl) )
		return DeleteFolderLink( pidl, bVerify );
	return FALSE;
}

bool CShellFolder::ConfirmDelete(LPITEMIDLIST *aPidls)
{
	int i, iFolders=0, iFiles=0;

	// Count the folders to delete
	for(i=0; aPidls[i]; i++)
	{
		if( CPidlManager::IsFile(aPidls[i]) )
			iFiles++;
		else
			iFolders++;
	}

	TCHAR msg[100] = {0};
	TCHAR szCaption[50] = {0};
	TCHAR szQuestion[200] = {0};
	UINT nCaption, nQuestion;
	int iCount1=0, iCount2=0;

	if(iFolders > 0)
	{
		if(iFiles == 0)
		{
			nCaption	= IDS_DELETEFOLDERS_CAPTION;
			nQuestion	= IDS_DELETEFOLDERS;
			iCount1		= iFolders;
		}
		else
		{
			nCaption	= IDS_DELETEITEMS_CAPTION;
			nQuestion	= IDS_DELETEITEMS;
			iCount1		= iFolders;
			iCount2		= iFiles;
		}
	}
	else
	{
		nCaption	= IDS_DELETEFILES_CAPTION;
		nQuestion	= IDS_DELETEFILES;
		iCount1		= iFiles;
	}

	::LoadString(g_hInst, nCaption, szCaption, ARRAYSIZE(szCaption));
	::LoadString(g_hInst, nQuestion, szQuestion, ARRAYSIZE(szQuestion));
	wsprintf(msg, szQuestion, iCount1, iCount2);

	int nResult = ::MessageBox( NULL/*hWnd*/,msg, szCaption,MB_YESNO);

	return (IDYES == nResult);
}

void CShellFolder::Delete(LPITEMIDLIST *aPidls)
{
	if( ConfirmDelete(aPidls) )
	{
		for(int i=0; aPidls[i]; i++)
		{
			DeletePidl( aPidls[i], FALSE );
		}
	}
}

BOOL CShellFolder::RenameFile(LPCITEMIDLIST pidl, LPCTSTR pszName)
{
	if( !CPidlManager::IsFile(pidl) )
		return false;

	CPidl cpidl(pidl);
	LPPIDLDATA pData = cpidl.GetData();

	TCHAR szOldPath[MAX_PATH] = {0};
	lstrcpy(szOldPath, pData->fileData.szPath);

	TCHAR szNewPath[MAX_PATH] = {0};
	lstrcpy(szNewPath, pData->fileData.szPath);
	LPSTR p = strrchr(szNewPath, '\\');
	*(++p) = 0;
	lstrcat(szNewPath, pszName);

	SHFILEOPSTRUCT shfos = {0};

//	shfos.hwnd = ;
	shfos.wFunc		= FO_RENAME; //FO_COPY FO_DELETE FO_MOVE FO_RENAME 
	shfos.pFrom		= szOldPath;
	shfos.pTo		= szNewPath;
	shfos.fFlags	= FOF_ALLOWUNDO; //FOF_ALLOWUNDO FOF_NOCONFIRMATION 

	int result = ::SHFileOperation(&shfos);
	if( 0 == result )
	{
		lstrcpy(pData->fileData.szPath, szNewPath);
		return true;
	}

	return false;
}

BOOL CShellFolder::RenameSubFolder(LPCITEMIDLIST pidl, LPCTSTR pszName)
{
	BOOL bResult = FALSE;

	LPITEMIDLIST pidlNew = g_pPidlMgr->Copy(pidl);
	LPPIDLDATA pDataNew = CPidlManager::GetDataPointer(pidlNew);
	lstrcpy(pDataNew->szName, pszName);

	MSXML2::IXMLDOMNodePtr ptrNode = m_pidlFQ.GetNode();
	if(NULL == g_pConfigXML->GetSubfolder(ptrNode, pDataNew->szName))
	{
		LPITEMIDLIST pidlFQNew = CreateFQPidl(pidlNew);
		LPITEMIDLIST pidlFQOld = CreateFQPidl(pidl);
		if( g_pConfigXML->SaveFolder(pidlFQOld, pidlFQNew) )
		{

			//TRACE_PIDL_PATH("CShellView::OnEndLabelEdit pidlFQOld: %s\n", pidlFQOld);
			//TRACE_PIDL_PATH("CShellView::OnEndLabelEdit pidlFQNew: %s\n", pidlFQNew);

			::SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_IDLIST, pidlFQOld, pidlFQNew);

			bResult = TRUE;
		}

		g_pPidlMgr->Delete(pidlFQOld);
		g_pPidlMgr->Delete(pidlFQNew);
	}

	g_pPidlMgr->Delete(pidlNew);

	return bResult;
}

BOOL CShellFolder::Rename(LPCITEMIDLIST pidl, LPCTSTR pszName)
{
	if( CPidlManager::IsFile(pidl) )
	{
		return RenameFile(pidl, pszName);
	}
	else
	{
		return RenameSubFolder(pidl, pszName);
	}
}

void CShellFolder::RemoveFiletype(LPCTSTR ext)
{
//	TRACE_PIDL_PATH("pidlParentFQOld: %s\n", m_pidlFQ);


	MSXML2::IXMLDOMNodePtr ptrNode = m_pidlFQ.GetNode();
	LPPIDLDATA pData = m_pidlFQ.GetData();
	g_pConfigXML->GetFolderInfo(pData, ptrNode);

	LPTSTR pszQuery = pData->searchData.szQuery;

	if(0 == lstrlen(pszQuery))
		lstrcpy(pszQuery, pData->szName);

	wsprintf(pszQuery, "%s -filetype:%s", pszQuery, ext);

	g_pConfigXML->SaveFolder( m_pidlFQ );
}

void CShellFolder::AddExtensionSubfolder(LPCTSTR ext)
{
	// Add new folder
	TCHAR name[50] = { 0 };
	wsprintf(name, "%s files", ext);
	CPidl pidlNewFQ = CreateSubfolder(NULL, name);
	//TRACE_PIDL_PATH("AddExtensionSubfolder(%s)\n", pidlNewFQ);

	CPidl pidlOldFQ = m_pidlFQ + pidlNewFQ.GetRelative();

	// Get this (parent) folder's query
	MSXML2::IXMLDOMNodePtr ptrNode = m_pidlFQ.GetNode();
	LPPIDLDATA pData = m_pidlFQ.GetData();
	g_pConfigXML->GetFolderInfo(pData, ptrNode);
	LPTSTR pszParentQuery = pData->searchData.szQuery;

	if(0 == lstrlen(pszParentQuery))
		pszParentQuery = pData->szName;

	// Set new folder's query
	LPPIDLDATA pDataNew = pidlNewFQ.GetData();
	wsprintf(pDataNew->searchData.szQuery, "%s filetype:%s", pszParentQuery, ext);

	// Save new folder
	g_pConfigXML->SaveFolder( pidlNewFQ );

//	::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlNewFQ, NULL);
}

STDMETHODIMP CShellFolder::MoveCopyItems( CShellFolder *psfSource, 
                                          LPITEMIDLIST *aPidls, 
                                          UINT uCount,
                                          BOOL fMove)
{
//	TRACE_PIDL_PATH("CShellFolder::MoveCopyItems(src: %s)\n", &psfSource->m_pidlFQ);
//	TRACE_PIDL_PATH("CShellFolder::MoveCopyItems(dst: %s)\n", &m_pidlFQ);

	for(UINT i=0; i<uCount; i++)
	{
		CPidl cPidlSrc = psfSource->CreateFQPidl( aPidls[i] );

//		TRACE_PIDL_PATH("CShellFolder::MoveCopyItems(%s)\n", &pidlSrc);

		LPITEMIDLIST src = cPidlSrc;
		LPITEMIDLIST dst = m_pidlFQ;

		if(fMove)
		{
			if( g_pConfigXML->MoveFolder(src, dst) )
			{
				::SHChangeNotify(SHCNE_RMDIR, SHCNF_IDLIST, src, NULL);

				CPidl cPidlNewFQ = CreateFQPidl( aPidls[i] );
//				TRACE_PIDL_PATH("CShellFolder::MoveCopyItems(%s)\n", &cPidlNewFQ);
				::SHChangeNotify(SHCNE_MKDIR, SHCNF_IDLIST, cPidlNewFQ, NULL);
				g_pViewList->Refresh();
			}
		}
		else
		{
			_RPTF0(_CRT_WARN, "Copy not implemented\n");
		}
	}
	
	return S_OK;
}

void CShellFolder::ShowOnlyExtension(LPCTSTR ext)
{
	MSXML2::IXMLDOMNodePtr ptrNode = m_pidlFQ.GetNode();
	LPPIDLDATA pData = m_pidlFQ.GetData();
	g_pConfigXML->GetFolderInfo(pData, ptrNode);

	LPTSTR pszQuery = pData->searchData.szQuery;

	if(0 == lstrlen(pszQuery))
		lstrcpy(pszQuery, pData->szName);

	wsprintf(pszQuery, "%s filetype:%s", pszQuery, ext);

	g_pConfigXML->SaveFolder( m_pidlFQ );
}

void CShellFolder::ClearFolderSearch(LPCITEMIDLIST pidl)
{
	if(NULL == pidl)
	{
		LPPIDLDATA pData = m_pidlFQ.GetData();
		memset(pData->searchData.szQuery, 0, sizeof(pData->searchData.szQuery));
		g_pConfigXML->SaveFolder( m_pidlFQ );
	}
	else
	{
		CPidl pidlFQ = CreateFQPidl(pidl);
		
		LPPIDLDATA pData = pidlFQ.GetData();
		memset(pData->searchData.szQuery, 0, sizeof(pData->searchData.szQuery));
		g_pConfigXML->SaveFolder( pidlFQ );

		g_pViewList->Refresh();
	}
}

LPITEMIDLIST CShellFolder::CreateSubfolder(LPCITEMIDLIST pidlParent,
										   LPCTSTR pszName)
{
	MSXML2::IXMLDOMNodePtr ptrChildNode;
	LPITEMIDLIST pidlNew = NULL;
	LPITEMIDLIST pidlFQ = NULL;
	BOOL bUnderThisFolder = FALSE;

	MSXML2::IXMLDOMNodePtr ptrNode = m_pidlFQ.GetNode();

	ptrChildNode = g_pConfigXML->CreateFolder( pszName, ptrNode );

	if(NULL == pidlParent)
	{
		pidlNew = g_pPidlMgr->CreateFolder(ptrChildNode);
		bUnderThisFolder = TRUE;
	}
	else
	{
		CPidl pidlLocal(pidlParent);
		pidlNew = pidlLocal + g_pPidlMgr->CreateFolder(ptrChildNode);
	}

	pidlFQ = CreateFQPidl(pidlNew);

	::SHChangeNotify(SHCNE_MKDIR, SHCNF_IDLIST, pidlFQ, NULL);
	TRACE_PIDL_PATH("CShellFolder::CreateSubfolder(%s)\n", pidlFQ);

	if(!bUnderThisFolder)
	{
		// Just add the folder, no need for full reload!
		g_pViewList->Refresh();
	}
	else
	{
		//Get active listview and send it LVN_BEGINLABELEDIT
		CShellView *pView = g_pViewList->GetActiveView();
		if(pView)
		{
			pView->AddNewFolder(pidlFQ);
		}
	}

	g_pPidlMgr->Delete(pidlNew);

	return pidlFQ;
}

LPITEMIDLIST CShellFolder::CreateNewFolder(LPCITEMIDLIST pidlParent)
{
	TCHAR szName[100] = {0};
	LPITEMIDLIST pidlFQ = NULL;
	LPITEMIDLIST pidlNew = NULL;
	MSXML2::IXMLDOMNodePtr ptrChildNode;

	MSXML2::IXMLDOMNodePtr ptrNode = m_pidlFQ.GetNode();

	::LoadString(g_hInst, IDS_NEWFOLDER, szName, ARRAYSIZE(szName));


	TCHAR szTemp[100] = {0};
	lstrcpy(szTemp, szName);
	int i = 0;
	while(NULL != g_pConfigXML->GetSubfolder(ptrNode, szTemp))
	{
		wsprintf(szTemp, "%s (%d)", szName, ++i);
	}


	ptrChildNode = g_pConfigXML->CreateFolder( szTemp, ptrNode );

	if(pidlParent)
	{
		CPidl pidlLocal(pidlParent);
		pidlNew = pidlLocal + g_pPidlMgr->CreateFolder(ptrChildNode);
	}
	else
	{
		pidlNew = g_pPidlMgr->CreateFolder(ptrChildNode);
	}

	pidlFQ = CreateFQPidl(pidlNew);

	::SHChangeNotify(SHCNE_MKDIR, SHCNF_IDLIST, pidlFQ, NULL);
	TRACE_PIDL_PATH("CShellFolder::CreateNewFolder(%s)\n", pidlFQ);

	//Get active listview and send it LVN_BEGINLABELEDIT
	CShellView *pView = g_pViewList->GetActiveView();
	if(pView)
	{
		pView->AddNewFolder(pidlFQ);
	}

	g_pPidlMgr->Delete(pidlNew);

	return pidlFQ;
}

void CShellFolder::DisplayVersion()
{
	CFolderPropertiesDlg *pDlg;
	pDlg = new CFolderPropertiesDlg(m_pSFParent);
	if(pDlg)
		pDlg->Show();
/*
	CVersionInfoDlg *pDlg;
	pDlg = new CVersionInfoDlg(m_pSFParent);
	if(pDlg)
		pDlg->Show();
*/
/*
//	::MessageBeep(MB_OK);

	TCHAR szFullPath[MAX_PATH] = {0};
	::GetModuleFileName(g_hInst, szFullPath, MAX_PATH);

	DWORD dwVerInfoSize;
	DWORD dwVerHnd=0;
	dwVerInfoSize = ::GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	if(dwVerInfoSize)
	{
		LPSTR   lpstrVffInfo;
		HANDLE  hMem;
		hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo = (LPSTR)::GlobalLock(hMem);
		::GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);

		UINT uVersionLen = 0;
		LPSTR lpVersion = NULL;
		TCHAR szGetName[256];
		lstrcpy(szGetName, "\\StringFileInfo\\040904b0\\");	 
		lstrcat(szGetName, TEXT("ProductVersion"));
		BOOL bRetCode = ::VerQueryValue(
			(LPVOID)lpstrVffInfo,
			(LPSTR)szGetName,
			(LPVOID*)&lpVersion,
			(UINT *)&uVersionLen);

		if(bRetCode)
		{
			TCHAR szMessage[256] = {0};
			TCHAR szPath[MAX_PATH] = {0};
			CSettings::GetXmlFilePath(szPath, ARRAYSIZE(szPath));
			*strrchr(szPath, '\\') = 0;
			TCHAR szTemp[200] = {0};
			::LoadString(g_hInst, IDS_VERSIONINFO, szTemp, ARRAYSIZE(szTemp));
			wsprintf(szMessage, szTemp, lpVersion, szPath);
			::MessageBox(
				NULL,//hWnd
				szMessage,
				TEXT("About zenFolders"),
				MB_OK | MB_ICONINFORMATION);
		}

		::GlobalUnlock(hMem);
		::GlobalFree(hMem);
	}
*/
}

void CShellFolder::OpenFolder(HWND hwnd, LPCITEMIDLIST pidl, BOOL bExplore)
{
	if(NULL == pidl)
		return;

	CPidl pidlFQ = CreateFQPidl(pidl);

//	TRACE_PIDL_PATH("CContextMenu::OnOpenFolder(%s)\n", pidlFQ);

	SHELLEXECUTEINFO  sei = {0};
	sei.cbSize		= sizeof(sei);
	sei.fMask		= SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
	sei.lpIDList	= pidlFQ;
	sei.lpClass		= TEXT("folder");
	sei.hwnd		= hwnd;
	sei.nShow		= SW_SHOWNORMAL;
	sei.lpVerb		= bExplore ? TEXT("explore") : TEXT("open");
	::ShellExecuteEx( &sei );
}

void CShellFolder::Execute(LPCITEMIDLIST pidl)
{
	if( (NULL == pidl) || (!CPidlManager::IsFile(pidl)) )
		return;

	LPPIDLDATA pData = CPidlManager::GetDataPointer( pidl );

	if( NULL == pData )
		return;

	LPITEMIDLIST pidlFS = pData->fileData.pidlFS;

	if( NULL == pidlFS )
		return;

	SHELLEXECUTEINFO  sei = {0};
	sei.cbSize		= sizeof(SHELLEXECUTEINFO);
	sei.fMask		= SEE_MASK_NOCLOSEPROCESS | SEE_MASK_IDLIST; 
	sei.lpIDList	= pidlFS;
	sei.nShow		= SW_SHOWNORMAL;
	::ShellExecuteEx( &sei );
}

void CShellFolder::OpenContainingFolder(LPCITEMIDLIST pidl)
{
	if( (NULL == pidl) || !CPidlManager::IsFile(pidl) )
		return;

	LPPIDLDATA pData = CPidlManager::GetDataPointer( pidl );
	LPFILEDATA pFileData = &pData->fileData;
	TCHAR szPath[MAX_PATH] = {0};
	lstrcpy(szPath, pFileData->szPath);
	LPSTR p = strrchr(szPath, '\\');
	if(p)
	{
		*(++p) = 0;

		SHELLEXECUTEINFO  sei = {0};
		sei.cbSize		= sizeof(sei);
		sei.fMask		= SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
		sei.lpFile		= szPath;
		sei.lpClass		= TEXT("folder");
		sei.nShow		= SW_SHOWNORMAL;
		sei.lpVerb		= TEXT("open");
		::ShellExecuteEx( &sei );
	}
}

void CShellFolder::SaveListViewStyle(LONG style)
{
	if( IsRoot() )
		return;

	LPPIDLDATA pData = m_pidlFQ.GetData();
	if(pData)
	{
		LPSEARCHDATA pSearchData = &pData->searchData;
		pSearchData->viewStyle = style;
		g_pConfigXML->SaveFolder( m_pidlFQ );
	}
}

LONG CShellFolder::GetListViewStyle()
{
	if( !IsRoot() )
	{
		MSXML2::IXMLDOMNodePtr ptrNode = m_pidlFQ.GetNode();
		LPPIDLDATA pData = m_pidlFQ.GetData();
		if(pData)
		{
			g_pConfigXML->GetFolderInfo(pData, ptrNode);
			LPSEARCHDATA pSearchData = &pData->searchData;
			return pSearchData->viewStyle;
		}
	}
	return LVS_REPORT2;
}

void CShellFolder::AddFolderLink(LPCTSTR lpszPath)
{
	g_pConfigXML->CreateFolderLink(lpszPath, m_pidlFQ.GetNode());

	::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, m_pidlFQ, NULL);
	g_pViewList->Refresh();
}

void CShellFolder::AddFileLink(LPCTSTR lpszPath)
{
	g_pConfigXML->CreateFileLink(lpszPath, m_pidlFQ.GetNode());

	::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, m_pidlFQ, NULL);
	//SHCNE_CREATE
	g_pViewList->Refresh();
}
