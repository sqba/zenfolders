// icons.cpp: implementation of the CIconList class.
//
//////////////////////////////////////////////////////////////////////

#include <crtdbg.h>
#include <shlobj.h>
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

CIconList::CIconList(HINSTANCE hInst)
{
	m_hInst = hInst;

	m_iShellIconIndex = 0;

	memset(m_ShellIcons, 0, sizeof(m_ShellIcons));

	m_pFileTypes = new FILETYPE();
	memset(m_pFileTypes, 0, sizeof(FILETYPE));

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

//	_RPTF0(_CRT_WARN, "Loaded shell icons\n");
}

CIconList::~CIconList()
{
	int count = 0;

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
			count++;
		}
	}

	LPFILETYPE tmp = m_pFileTypes;
	LPFILETYPE next;
	while(tmp)
	{
		next = tmp->next;
		::DestroyIcon(tmp->hIconSmall);
		::DestroyIcon(tmp->hIconLarge);
		delete tmp;
		tmp = next;
		count++;
	}

//	_RPTF1(_CRT_WARN, "Destroyed %d icons\n", count);
}


//////////////////////////////////////////////////////////////////////
// Public functions
//////////////////////////////////////////////////////////////////////

HICON CIconList::GetIconLarge(UINT index)
{
	return ImageList_GetIcon(m_himlLarge, index, ILD_TRANSPARENT);
}

HICON CIconList::GetIconSmall(UINT index)
{
	return ImageList_GetIcon(m_himlSmall, index, ILD_TRANSPARENT);
}

HIMAGELIST CIconList::GetListLarge()
{
	return m_himlLarge;
}

HIMAGELIST CIconList::GetListSmall()
{
	return m_himlSmall;
}

int CIconList::GetIconIndex(LPCTSTR pszPath)
{
	LPFILETYPE tmp = m_pFileTypes;
	while(tmp)
	{
		if(0 == lstrcmpi(tmp->szPath, pszPath))
		{
			return tmp->index;
		}
		tmp = tmp->next;
	}

	return AddAsociatedIcon(pszPath);
}


//////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////

HIMAGELIST CIconList::CreateImageList(int size)
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

LPFILETYPE CIconList::CreateNewFileType(LPCTSTR pszPath)
{
	LPFILETYPE tmp = m_pFileTypes;
	LPFILETYPE last = m_pFileTypes;

	while(tmp)
	{
		last = tmp;
		tmp = tmp->next;
	}

	if(strlen(last->szPath) > 0)
	{
		LPFILETYPE newType = new FILETYPE();
		memset(newType, 0, sizeof(FILETYPE));
		last->next = newType;
		last = newType;
	}

	lstrcpyn(last->szPath, pszPath, ARRAYSIZE(last->szPath));

	return last;
}

bool CIconList::LoadShellIcons(HIMAGELIST himl, bool bSmall)
{
	// 'Dummy' icon, somehow resolves the problem with psd files
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

bool CIconList::AddShellIcon(HIMAGELIST himl, bool bSmall, int nIconIndex)
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

int CIconList::AddAsociatedIcon(LPCTSTR pszPath)
{
	LPFILETYPE newType = CreateNewFileType(pszPath);

	HICON hIconSmall = GetAsociatedIcon(pszPath, true);
	HICON hIconLarge = GetAsociatedIcon(pszPath, false);

	newType->hIconSmall = hIconSmall;
	newType->hIconLarge = hIconLarge;

	int iSmall = ImageList_AddIcon(m_himlSmall, hIconSmall);
	int iLarge = ImageList_AddIcon(m_himlLarge, hIconLarge);

	_ASSERT(iSmall == iLarge);

	newType->index = iSmall;

	return newType->index;
}

HICON CIconList::GetAsociatedIcon(LPCTSTR pszPath, bool bSmall)
{
	LPCTSTR pszExtension = CString::GetExtension(pszPath);
	if(0 == lstrcmpi(pszExtension, ".ico"))
	{
		HICON hIcon = LoadIcon(pszPath, true);
		if(hIcon)
		{
			return hIcon;
		}
	}

	// Problem: when path points to a .psd file,
	// after the call to SHGetFileInfo, image lists
	// get confused and when asked for a folder icon,
	// they return psd icon. This happens even if I
	// return NULL ignoring what SHGetFileInfo returned.

	DWORD dwFileAttributes = 0;
	SHFILEINFO shfi = {0};
	DWORD_PTR result = 0;
	UINT uFlags = 0;

//	dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	uFlags = SHGFI_ICON;
	uFlags |= (bSmall ? SHGFI_SMALLICON : SHGFI_LARGEICON);
//	uFlags |= SHGFI_USEFILEATTRIBUTES;
//	uFlags |= SHGFI_ATTR_SPECIFIED;
//	uFlags |= SHGFI_SHELLICONSIZE;

	result = ::SHGetFileInfo(
		pszPath,			// pszPath
		dwFileAttributes,	// dwFileAttributes
		&shfi,				// psfi
		sizeof(shfi),		// cbFileInfo
		uFlags);			// uFlags

	if(shfi.hIcon)
	{
		return shfi.hIcon;
	}
	else
	{
		return ExtractShellIcon(SI_DEF_DOCUMENT, bSmall);
	}
}

HICON CIconList::ExtractShellIcon(int nIndex, bool bSmall)
{
	int nIcons = 0;
    HICON hIcon = NULL;

    nIcons = ::ExtractIconEx(
        TEXT("SHELL32.DLL"),		// lpszFile
        nIndex,						// nIconIndex
		(bSmall ? NULL : &hIcon),	// phiconLarge
        (bSmall ? &hIcon : NULL),	// phiconSmall
        1);							// nIcons

    return nIcons ? hIcon : NULL;

}

HICON CIconList::LoadIcon(LPCTSTR pszPath, bool bSmall)
{
	int cx, cy;

	cx = cy = bSmall ? 16 : 32;
	
	return (HICON)::LoadImage(
		NULL,
		pszPath,
		IMAGE_ICON,
		cx, cy,
		LR_LOADFROMFILE);
}
