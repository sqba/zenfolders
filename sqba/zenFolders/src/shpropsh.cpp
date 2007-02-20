// shpropsh.cpp: implementation of the CShellPropSheetExt class.
//
//////////////////////////////////////////////////////////////////////

#include "shpropsh.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShellPropSheetExt::CShellPropSheetExt()
{

}

CShellPropSheetExt::~CShellPropSheetExt()
{

}

///////////////////////////////////////////////////////////////////////////
//
// IShellPropSheetExt Implementation
//

/**************************************************************************
IShellPropSheetExt::AddPages
	Adds one or more pages to a property sheet that the Shell displays for
	a file object. When it is about to display the property sheet,
	the Shell calls this method for each property sheet handler registered
	to the file type.

Parameters
	pfnAddPage [in]	Pointer to a function that the property sheet handler
					calls to add a page to the property sheet. The function
					takes a property sheet handle returned by the
					CreatePropertySheetPage function and the lParam
					parameter passed to the AddPages method.
	lParam [in]		Parameter to pass to the function specified by the
					pfnAddPage method.

Return Value
	Returns S_OK if successful, or an error value otherwise.
	Version 4.71 If successful, returns a one-based index to specify
	the page that should be initially displayed. See the remarks for
	more information.
	If the method fails, an OLE-defined error code is returned.
**************************************************************************/
HRESULT CShellPropSheetExt::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage,
									 LPARAM lParam)
{
	return E_NOTIMPL;
}

/**************************************************************************
IShellPropSheetExt::ReplacePage
	Replaces a page in a property sheet for a Control Panel object.

Parameters
	uPageID				Not used.
						Microsoft Windows XP and earlier: A type EXPPS
						identifier of the page to replace. The values for
						this parameter for Control Panels can be found in
						the Cplext.h header file.
	pfnReplacePage [in] Pointer to a function that the property sheet
						handler calls to replace a page to the property sheet.
						The function takes a property sheet handle returned
						by the CreatePropertySheetPage function and the
						lParam parameter passed to the ReplacePage method. 
	lParam [in]			Parameter to pass to the function specified by the
						pfnReplacePage parameter. 

Return Value
	Returns S_OK if successful, or an error value otherwise.
**************************************************************************/
HRESULT CShellPropSheetExt::ReplacePage(UINT uPageID,
										LPFNADDPROPSHEETPAGE pfnReplacePage,
										LPARAM lParam)
{
	return E_NOTIMPL;
}
