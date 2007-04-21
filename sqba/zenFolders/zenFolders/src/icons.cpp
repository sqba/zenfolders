// icons.cpp: implementation of the CIcons class.
//
//////////////////////////////////////////////////////////////////////

//#include <crtdbg.h>
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

	memset(m_ShellIcons, 0, sizeof(m_ShellIcons));

	m_pFileTypes = new tagFileType();
	memset(m_pFileTypes, 0, sizeof(tagFileType));

	m_himlSmall = CreateImageList(16);
	if(m_himlSmall)
	{
		LoadShellIcons(m_himlSmall, true);
	}

	m_himlLarge = CreateImageList(32);
	if(m_himlLarge)
	{
		LoadShellIcons(m_himlLarge, false);
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
		sizeof(newType->szExtension));
	return newType;
}

void CIcons::LoadShellIcons(HIMAGELIST himl, bool bSmall)
{
	AddShellIcon(himl, SI_FOLDER_CLOSED, bSmall);	// ICON_INDEX_FOLDER
	AddShellIcon(himl, SI_FOLDER_OPEN, bSmall);		// ICON_INDEX_FOLDEROPEN
	AddShellIcon(himl, SI_DEF_DOCUMENT, bSmall);	// ICON_INDEX_FILE
}

void CIcons::AddShellIcon(HIMAGELIST himl, int nIndex, bool bSmall)
{
	static int index = 0;
	HICON hIcon;
	hIcon = ExtractShellIcon(nIndex, bSmall);
	ImageList_AddIcon(himl, hIcon);
	if(index < ARRAYSIZE(m_ShellIcons))
	{
		m_ShellIcons[index++] = hIcon;
	}
}

int CIcons::AddAsociatedIcon(LPCTSTR pszPath)
{
	HICON hIconSmall = GetAsociatedIcon(pszPath, true);
	HICON hIconLarge = GetAsociatedIcon(pszPath, false);

	if((NULL == hIconSmall) && (NULL == hIconLarge))
		return -1;

	if(NULL == hIconSmall)
		hIconSmall = (HICON)::LoadImage(
			m_hInst,
			MAKEINTRESOURCE(IDI_FILE),
			IMAGE_ICON,
			16, 16,
			LR_DEFAULTCOLOR);

	if(NULL == hIconLarge)
		hIconLarge = (HICON)::LoadImage(
			m_hInst,
			MAKEINTRESOURCE(IDI_FILE),
			IMAGE_ICON,
			32, 32,
			LR_DEFAULTCOLOR);

	LPCTSTR pszExtension = CString::GetExtension(pszPath);

	tagFileType *newType = CreateNewFileType(pszExtension);

	newType->hIconSmall = hIconSmall;
	newType->hIconLarge = hIconLarge;

	int s = ImageList_AddIcon(m_himlSmall, hIconSmall);
	int l = ImageList_AddIcon(m_himlLarge, hIconLarge);
	// assert(s == l);
	newType->index = s;

	return newType->index;
}

HICON CIcons::GetAsociatedIcon(LPCTSTR pszPath, bool bSmall)
{
	DWORD dwAttributes =
		SHGFI_ICON
		| SHGFI_USEFILEATTRIBUTES
		| (bSmall ? SHGFI_SMALLICON : SHGFI_LARGEICON);

	SHFILEINFO shfi = {0};
	DWORD_PTR result = 0;

	result = ::SHGetFileInfo(
		pszPath,				// pszPath
		FILE_ATTRIBUTE_NORMAL,	// dwFileAttributes
		&shfi,					// psfi
		sizeof(shfi),			// cbFileInfo
		dwAttributes);			// uFlags

	return result ? shfi.hIcon : NULL;
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
        TEXT("SHELL32.DLL"),	// lpszFile
        nIndex,					// nIconIndex
		bSmall ? NULL : &hIcon,	// phiconLarge
        bSmall ? &hIcon : NULL,	// phiconSmall
        1);						// nIcons

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
