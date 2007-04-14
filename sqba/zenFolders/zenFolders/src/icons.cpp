// icons.cpp: implementation of the CIcons class.
//
//////////////////////////////////////////////////////////////////////

#include "icons.h"
#include "util/string.h"
#include "resource.h"
#include "sysicons.h"


#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIcons::CIcons(HINSTANCE hInst)
{
	m_hInst = hInst;

	m_himlSmall = CreateImageList(16);
	if(m_himlSmall)
	{
		AddDefaultIcons(m_himlSmall, true);
	}

	m_himlLarge = CreateImageList(32);
	if(m_himlLarge)
	{
		AddDefaultIcons(m_himlLarge, false);
	}

	memset(m_SystemIcons, 0, sizeof(m_SystemIcons));

	m_pExtensions = new tagExtension();
	m_pExtensions->next = NULL;
	m_pExtensions->index = 0;
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

	for(int i=0; i<ARRAYSIZE(m_SystemIcons); i++)
	{
		if(m_SystemIcons[i])
			::DestroyIcon( m_SystemIcons[i] );
	}

	tagExtension *tmp = m_pExtensions;
	tagExtension *next;
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

	tagExtension *tmp = m_pExtensions;
	while(tmp->next)
	{
		if(0 == lstrcmpi(tmp->szExtension, pszExtension))
			return tmp->index;
		tmp = tmp->next;
	}
	// not found
	return AddIcon(pszPath);
}

tagExtension *CIcons::CreateNewExtension(LPCTSTR pszExtension)
{
	tagExtension *tmp = m_pExtensions;
	tagExtension *last = m_pExtensions;

	while(tmp->next)
	{
		last = tmp;
		tmp = tmp->next;
	}

	tagExtension *newExtension = new tagExtension();
	last->next = newExtension;
	newExtension->next = NULL;

	lstrcpyn(
		newExtension->szExtension,
		pszExtension,
		sizeof(newExtension->szExtension));
	return newExtension;
}

void CIcons::AddDefaultIcons(HIMAGELIST himl, bool bSmall)
{
	// This seems to fix the icon problem
	AddShellIcon(himl, SI_FOLDER_CLOSED, bSmall);

	AddShellIcon(himl, SI_FOLDER_CLOSED, bSmall);
	AddShellIcon(himl, SI_FOLDER_OPEN, bSmall);
	AddShellIcon(himl, SI_DEF_DOCUMENT, bSmall);
	//AddShellIcon(himl, SI_DEF_APPLICATION, bSmall);
}

void CIcons::AddShellIcon(HIMAGELIST himl, int nIndex, bool bSmall)
{
	static int index = 0;
	HICON hIcon;
	hIcon = ExtractShellIcon(nIndex, bSmall);
	ImageList_AddIcon(himl, hIcon);
	if(index < ARRAYSIZE(m_SystemIcons))
	{
		m_SystemIcons[index++] = hIcon;
	}
}

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

int CIcons::AddIcon(LPCTSTR pszPath)
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

	tagExtension *newExtension = CreateNewExtension(pszExtension);

	newExtension->hIconSmall = hIconSmall;
	newExtension->hIconLarge = hIconLarge;

	int s = ImageList_AddIcon(m_himlSmall, hIconSmall);
	int l = ImageList_AddIcon(m_himlLarge, hIconLarge);
	// assert(s == l);
	newExtension->index = s;

	return newExtension->index;
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

	return shfi.hIcon;
}

HIMAGELIST CIcons::CreateImageList(int size)
{
	int cx, cy;
	cx = cy = size;
	UINT flags = ILC_COLORDDB | ILC_MASK;
	return ImageList_Create(cx, cy, flags, 4, 0);
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

    return hIcon;
}
