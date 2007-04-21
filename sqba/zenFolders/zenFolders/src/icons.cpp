// icons.cpp: implementation of the CIcons class.
//
//////////////////////////////////////////////////////////////////////

#include <crtdbg.h>
#include "icons.h"
#include "util/string.h"
#include "resource.h"
#include "sysicons.h"


#define ARRAYSIZE(a)		(sizeof(a)/sizeof(a[0]))

#define IMAGELIST_SIZE		20
#define IMAGELIST_GROWTH	5


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIcons::CIcons(HINSTANCE hInst)
{
	m_hInst = hInst;

	m_iShellIconIndex = 0;

	memset(m_ShellIcons, 0, sizeof(m_ShellIcons));

	m_pFileTypes = new tagFileType();
	memset(m_pFileTypes, 0, sizeof(tagFileType));

	m_himlSmall = CreateImageList(16);
	if(m_himlSmall)
	{
		if(!LoadShellIcons(m_himlSmall, true))
		{
			_RPTF0(_CRT_ERROR, "LoadShellIcons(small) failed\n");
		}
	}
	else
	{
		_RPTF0(_CRT_ERROR, "CreateImageList(16) failed\n");
	}

	m_himlLarge = CreateImageList(32);
	if(m_himlLarge)
	{
		if(!LoadShellIcons(m_himlLarge, false))
		{
			_RPTF0(_CRT_ERROR, "LoadShellIcons(large) failed\n");
		}
	}
	else
	{
		_RPTF0(_CRT_ERROR, "CreateImageList(32) failed\n");
	}
}

CIcons::~CIcons()
{
	if(m_himlSmall)
	{
		ImageList_Destroy(m_himlSmall);
	}
	
	if(m_himlLarge)
	{
		ImageList_Destroy(m_himlLarge);
	}

	for(int i=0; i<ARRAYSIZE(m_ShellIcons); i++)
	{
		if(m_ShellIcons[i])
		{
			::DestroyIcon( m_ShellIcons[i] );
		}
	}

	tagFileType *tmp = m_pFileTypes;
	tagFileType *next;
	while(tmp->next)
	{
		next = tmp->next;
		::DestroyIcon(tmp->hIconSmall);
		::DestroyIcon(tmp->hIconLarge);
		delete tmp;
		tmp = next;
	}
}


//////////////////////////////////////////////////////////////////////
// Public functions
//////////////////////////////////////////////////////////////////////

HICON CIcons::GetIconLarge(UINT index)
{
	return ImageList_GetIcon(m_himlLarge, index, ILD_TRANSPARENT);
}

HICON CIcons::GetIconSmall(UINT index)
{
	return ImageList_GetIcon(m_himlSmall, index, ILD_TRANSPARENT);
}

HIMAGELIST CIcons::GetListLarge()
{
	return m_himlLarge;
}

HIMAGELIST CIcons::GetListSmall()
{
	return m_himlSmall;
}

int CIcons::GetIconIndex(LPCTSTR pszPath)
{
	LPCTSTR pszExtension = CString::GetExtension(pszPath);

	tagFileType *tmp = m_pFileTypes;
	while(tmp->next)
	{
		if(0 == lstrcmpi(tmp->szExtension, pszExtension))
		{
			return tmp->index;
		}
		tmp = tmp->next;
	}

	// Icon not found
	return AddAsociatedIcon(pszPath);
}


//////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////

bool CIcons::LoadShellIcons(HIMAGELIST himl, bool bSmall)
{
	// 'Ghost' icon, somehow resolves the problem with psd files
	if(!AddShellIcon(himl, bSmall, SI_DEF_DOCUMENT))
		return false;

	// ICON_INDEX_FOLDER
	if(!AddShellIcon(himl, bSmall, SI_FOLDER_CLOSED))
		return false;

	// ICON_INDEX_FOLDEROPEN
	if(!AddShellIcon(himl, bSmall, SI_FOLDER_OPEN))
		return false;

	// ICON_INDEX_FILE
	if(!AddShellIcon(himl, bSmall, SI_DEF_DOCUMENT))
		return false;

	return true;
}

bool CIcons::AddShellIcon(HIMAGELIST himl, bool bSmall, int nIconIndex)
{
	HICON hIcon = ExtractShellIcon(nIconIndex, bSmall);
	if(NULL != hIcon)
	{
		if(-1 != ImageList_AddIcon(himl, hIcon))
		{
			m_ShellIcons[m_iShellIconIndex++] = hIcon;
			return true;
		}
	}
	_RPTF2(_CRT_ERROR, "AddShellIcon(himl, %d, %d) failed!\n", bSmall, nIconIndex);
	return false;
}

int CIcons::AddAsociatedIcon(LPCTSTR pszPath)
{
	HICON hIconSmall = GetAsociatedIcon(pszPath, true);
	HICON hIconLarge = GetAsociatedIcon(pszPath, false);

	if((NULL == hIconSmall) && (NULL == hIconLarge))
	{
		_RPTF1(_CRT_ERROR, "AddAsociatedIcon(%s) failed to extract both small and big icon!\n", pszPath);
		return -1;
	}

	if(NULL == hIconSmall)
	{
		_RPTF1(_CRT_WARN, "AddAsociatedIcon(%s) failed to extract small icon, adding default document icon\n", pszPath);
		hIconSmall = ExtractShellIcon(SI_DEF_DOCUMENT, true);
	}

	if(NULL == hIconLarge)
	{
		_RPTF1(_CRT_WARN, "AddAsociatedIcon(%s) failed to extract big icon, adding default document icon\n", pszPath);
		hIconLarge = ExtractShellIcon(SI_DEF_DOCUMENT, false);
	}

	LPCTSTR pszExtension = CString::GetExtension(pszPath);

//	_ASSERT(0 != lstrcmpi(pszExtension, ".psd"));

	tagFileType *newType = CreateNewFileType(pszExtension);

	newType->hIconSmall = hIconSmall;
	newType->hIconLarge = hIconLarge;

	int iSmall = ImageList_AddIcon(m_himlSmall, hIconSmall);
	int iLarge = ImageList_AddIcon(m_himlLarge, hIconLarge);

	_ASSERT(iSmall == iLarge);

	newType->index = iSmall;

	return newType->index;
}

HICON CIcons::GetAsociatedIcon(LPCTSTR pszPath, bool bSmall)
{
	// Problem: when path points to a .psd file,
	// after the call to SHGetFileInfo, image lists
	// get confused and when asked for a folder icon,
	// they return psd icon. This happens even if I
	// return NULL ignoring what SHGetFileInfo returned.

	SHFILEINFO shfi = {0};
	DWORD_PTR result = 0;

	DWORD dwAttributes =
		SHGFI_ICON
		| SHGFI_USEFILEATTRIBUTES
		| (bSmall ? SHGFI_SMALLICON : SHGFI_LARGEICON);

	result = ::SHGetFileInfo(
		pszPath,				// pszPath
		FILE_ATTRIBUTE_NORMAL,	// dwFileAttributes
		&shfi,					// psfi
		sizeof(SHFILEINFO),		// cbFileInfo
		dwAttributes);			// uFlags

	return result ? shfi.hIcon : NULL;
}

tagFileType *CIcons::CreateNewFileType(LPCTSTR pszExtension)
{
	tagFileType *tmp = m_pFileTypes;
	tagFileType *last = m_pFileTypes;

	while(tmp->next)
	{
		last = tmp;
		tmp = tmp->next;
	}

	tagFileType *newType = new tagFileType();
	memset(newType, 0, sizeof(tagFileType));
	last->next = newType;

	lstrcpyn(
		newType->szExtension,
		pszExtension,
		ARRAYSIZE(newType->szExtension));
	return newType;
}

HIMAGELIST CIcons::CreateImageList(int size)
{
	int cx, cy;
	cx = cy = size;
	return ImageList_Create(
		cx,
		cy,
		ILC_MASK | ILC_COLOR32,
		IMAGELIST_SIZE,
		IMAGELIST_GROWTH);
}

HICON CIcons::ExtractShellIcon(int nIndex, bool bSmall)
{
	int nIcons = 0;
    HICON hIcon = NULL;

    nIcons = ::ExtractIconEx(
        TEXT("SHELL32.DLL"),		// lpszFile
        nIndex,						// nIconIndex
		(bSmall ? NULL : &hIcon),	// phiconLarge
        (bSmall ? &hIcon : NULL),	// phiconSmall
        1);							// nIcons

	//_RPTF3(_CRT_WARN, "ExtractIconEx(%d, %d) returned %d\n", nIndex, bSmall, nIcons);

    return nIcons ? hIcon : NULL;
}
/*
void CIcons::AddIcon(HIMAGELIST himl, int iconId, int size)
{
	int cx, cy;
	
	cx = cy = size;
	
	if(himl)
	{
		HICON hIcon;

		hIcon = (HICON)::LoadImage(
			m_hInst,
			MAKEINTRESOURCE(iconId),
			IMAGE_ICON,
			cx, cy,
			LR_DEFAULTCOLOR);

		ImageList_AddIcon(himl, hIcon);
	}
}
*/
