// ShellExtInit.cpp: implementation of the CShellExtInit class.
//
//////////////////////////////////////////////////////////////////////

#include "shexinit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShellExtInit::CShellExtInit()
{

}

CShellExtInit::~CShellExtInit()
{

}

///////////////////////////////////////////////////////////////////////////
//
// IShellExtInit Implementation
//

/**************************************************************************
IShellExtInit::Initialize
	Initializes a property sheet extension, shortcut menu extension,
	or drag-and-drop handler.

Parameters
	pidlFolder [in]	Pointer to an ITEMIDLIST structure that uniquely
					identifies a folder. For property sheet extensions,
					this parameter is NULL. For shortcut menu extensions,
					it is the item identifier list for the folder that
					contains the item whose shortcut menu is being displayed.
					For nondefault drag-and-drop menu extensions, this
					parameter specifies the target folder. 
	pdtobj [in]		Pointer to an IDataObject interface object that can be
					used to retrieve the objects being acted upon. 
	hkeyProgID [in]	Registry key for the file object or folder type. 

Return Value
	Returns S_OK if successful, or an error value otherwise.
**************************************************************************/
HRESULT CShellExtInit::Initialize(ITEMIDLIST *pidlFolder,
								  IDataObject *pdtobj,
								  HKEY hkeyProgID)
{
	return E_NOTIMPL;
}
