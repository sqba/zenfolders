
#include "extricon.h"
#include "zenfolders.h"

extern HINSTANCE	g_hInst;
extern LPPIDLMGR	g_pPidlMgr;

CExtractIcon::CExtractIcon(CShellFolder *pSFParent, LPCITEMIDLIST pidl)
{
	m_pSFParent = pSFParent;

	m_pidl = g_pPidlMgr->Copy(pidl);
	
	m_ObjRefCount = 1;
	
	gAddRef(CEXTRACTICON);
}

CExtractIcon::~CExtractIcon()
{
	if(m_pidl)
	{
		g_pPidlMgr->Delete(m_pidl);
		m_pidl = NULL;
	}

	gRelease(CEXTRACTICON);
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

STDMETHODIMP CExtractIcon::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;
	
	//IUnknown
	if(IsEqualIID(riid, IID_IUnknown))
		*ppReturn = this;
	
	//IExtractIcon
	else if(IsEqualIID(riid, IID_IExtractIcon))
		*ppReturn = (IExtractIcon*)this;
	
	if(*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CExtractIcon::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CExtractIcon::Release()
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
// IExtractIcon Implementation
//

/**************************************************************************
IExtractIcon::Extract
	Extracts an icon image from the specified location.

Parameters
	pszFile [in]		Pointer to a null-terminated string specifying the
						icon location. 
	nIconIndex [in]		The index of the icon in the file pointed to by
						pszFile.
	phiconLarge [out]	Pointer to an HICON value that receives the handle
						to the large icon. This parameter may be NULL.
	phiconSmall [out]	Pointer to an HICON value that receives the handle
						to the small icon. This parameter may be NULL.
	nIconSize [in]		Desired size of the icon, in pixels. The low word
						contains the size of the large icon, and the high
						word contains the size of the small icon. The size
						specified can be the width or height. The width of
						an icon always equals its height.

Return Value
	Returns NOERROR if the function extracted the icon, or S_FALSE if
	the calling application should extract the icon.
**************************************************************************/
STDMETHODIMP CExtractIcon::Extract(LPCTSTR pszFile, 
								   UINT nIconIndex, 
								   HICON *phiconLarge, 
								   HICON *phiconSmall, 
								   UINT nIconSize)
{
	*phiconLarge = m_pSFParent->GetIconLarge(nIconIndex);
	*phiconSmall = m_pSFParent->GetIconSmall(nIconIndex);

	return S_OK;
}

/**************************************************************************
IExtractIcon::GetIconLocation
	Retrieves the location and index of an icon.

Parameters
	uFlags [in]			Flags. This parameter can be zero or one or more of
						the following values.
		GIL_ASYNC		A client sets this flag to discover whether or not
						the icon should be extracted asynchronously. If the
						icon can be rapidly extracted, this flag is usually
						ignored. If extraction will be time-consuming,
						GetIconLocation should return E_PENDING. See the
						Remarks for further discussion.
		GIL_DEFAULTICON	Retrieve information about the fallback icon.
						Fallback icons are usually used while the desired
						icon is extracted and added to the cache.
		GIL_FORSHELL	The icon is to be displayed in a Shell folder. 
		GIL_FORSHORTCUT	The icon is to be used to indicate a shortcut.
						However, the icon extractor should not apply the
						shortcut overlay as that will be done later.
						Shortcut icons are state-independent.
		GIL_OPENICON	The icon should be in the open state if both
						open- and closed-state images are available.
						If this flag is not specified, the icon should be
						in the normal or closed state. This flag is typically
						used for folder objects. 
	szIconFile [out]	Pointer to a buffer that receives the icon location.
						The icon location is a null-terminated string that
						identifies the file that contains the icon. 
	cchMax [in]			Size of the buffer, in characters, pointed to by
						szIconFile. 
	piIndex [out]		Pointer to an int that receives the index of the
						icon in the file pointed to by szIconFile. 
	pwFlags [out]		Pointer to a UINT value that receives zero or a
						combination of the following values:
		GIL_DONTCACHE	The physical image bits for this icon should not
						be cached by the caller.
		GIL_NOTFILENAME	The location is not a file name/index pair. Callers
						that decide to extract the icon from the location
						must call this object's IExtractIcon::Extract method
						to obtain the desired icon images. 
		GIL_PERCLASS	All objects of this class have the same icon.
						This flag is used internally by the Shell.
						Typical implementations of IExtractIcon do not
						require this flag because the flag implies that an
						icon handler is not required to resolve the icon
						on a per-object basis. The recommended method for
						implementing per-class icons is to register a
						DefaultIcon for the class. 
		GIL_PERINSTANCE	Each object of this class has its own icon.
						This flag is used internally by the Shell to handle
						cases like Setup.exe, where objects with identical
						names can have different icons. Typical implementations
						of IExtractIcon do not require this flag. 
		GIL_SIMULATEDOC	The caller should create a document icon using the
						specified icon.

Return Value
	Returns S_OK if the function returned a valid location, or S_FALSE if
	the Shell should use a default icon. If the GIL_ASYNC flag is set in
	uFlags, the method can return E_PENDING to indicate that icon
	extraction will be time consuming.
**************************************************************************/
STDMETHODIMP CExtractIcon::GetIconLocation(UINT uFlags, 
										   LPTSTR szIconFile, 
										   UINT cchMax, 
										   LPINT piIndex, 
										   LPUINT puFlags)
{
	//tell the shell to always call Extract
	*puFlags = GIL_NOTFILENAME;
	
	LPITEMIDLIST pidlRel = CPidlManager::GetLastItem(m_pidl);

	if( !CPidlManager::IsFile(pidlRel) )
	{
		if(uFlags & GIL_OPENICON)
			*piIndex = ICON_INDEX_FOLDEROPEN;
		else
			*piIndex = ICON_INDEX_FOLDER;
	}
	else
	{
		LPPIDLDATA pData = CPidlManager::GetDataPointer(pidlRel);
		if(NULL != pData)
		{
			*piIndex = m_pSFParent->GetIconIndex(pData->fileData.szPath);
			if(*piIndex >= -1)
				return S_OK;
		}
		*piIndex = ICON_INDEX_FILE;
	}
	
	return S_OK;
}
