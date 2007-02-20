// icons.cpp: implementation of the CIcons class.
//
//////////////////////////////////////////////////////////////////////

#include "icons.h"
#include "util/string.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIcons::CIcons(HINSTANCE hInst)
{
	m_hInst = hInst;

	m_himlSmall = CreateImageList(16);
	if(m_himlSmall)
		AddDefaultIcons(m_himlSmall, 16);

	m_himlLarge = CreateImageList(32);
	if(m_himlLarge)
		AddDefaultIcons(m_himlLarge, 32);

	m_pExtensions = new tagExtension();
	m_pExtensions->next = NULL;
	m_pExtensions->index = 0;
}

CIcons::~CIcons()
{
	if(m_himlSmall)
		ImageList_Destroy(m_himlSmall);
	
	if(m_himlLarge)
		ImageList_Destroy(m_himlLarge);

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

int CIcons::AddIcon(LPCTSTR pszPath)
{
	HICON hIconSmall = GetIcon(pszPath, true);
	HICON hIconLarge = GetIcon(pszPath, false);

	if((NULL == hIconSmall) && (NULL == hIconLarge))
		return -1;

	if(NULL == hIconSmall)
		hIconSmall = (HICON)LoadImage(
			m_hInst,
			MAKEINTRESOURCE(IDI_FILE),
			IMAGE_ICON,
			16, 16,
			LR_DEFAULTCOLOR);

	if(NULL == hIconLarge)
		hIconLarge = (HICON)LoadImage(
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

HICON CIcons::GetIcon(LPCTSTR pszPath, bool bSmall)
{
	DWORD dwAttributes = SHGFI_ICON | SHGFI_USEFILEATTRIBUTES;
	if(bSmall)
		dwAttributes |= SHGFI_SMALLICON;
	else
		dwAttributes |= SHGFI_LARGEICON;
	SHFILEINFO shfi;
	memset(&shfi,0,sizeof(shfi));
	SHGetFileInfo(
		pszPath, 
		FILE_ATTRIBUTE_NORMAL,
		&shfi,
		sizeof(shfi),
		dwAttributes);
	return shfi.hIcon;
}

void CIcons::AddDefaultIcons(HIMAGELIST himl, int size)
{
	AddIcon(himl, IDI_FOLDER,		size); // This seems to fix the icon problem
	AddIcon(himl, IDI_FOLDER,		size);
	AddIcon(himl, IDI_FOLDEROPEN,	size);
	AddIcon(himl, IDI_FILE,			size);
}

HIMAGELIST CIcons::CreateImageList(int size)
{
	int cx, cy;
	cx = cy = size;
	return ImageList_Create(cx, cy, ILC_COLORDDB | ILC_MASK, 4, 0);
}

void CIcons::AddIcon(HIMAGELIST himl, int iconId, int size)
{
	int cx, cy;
	
	cx = cy = size;
	
	if(himl)
	{
		HICON hIcon;
		hIcon = (HICON)LoadImage(
			m_hInst,
			MAKEINTRESOURCE(iconId),
			IMAGE_ICON,
			cx, cy,
			LR_DEFAULTCOLOR);
		ImageList_AddIcon(himl, hIcon);
	}
}







/*
void ExtractIcon()
{
	HICON hIconSmall, hIconLarge;
	int iIcons = ExtractIconEx( szIconPath, 0, &hIconLarge, &hIconSmall, 1 );
	ImageList_AddIcon(m_himlSmall, hIconSmall);
	ImageList_AddIcon(m_himlLarge, hIconLarge);
}
*/
/*
UINT ExtractIconEx(
  LPCTSTR lpszFile,        // file name
  int nIconIndex,          // icon index
  HICON *phiconLarge,      // large icon array
  HICON *phiconSmall,      // small icon array
  UINT nIcons              // number of icons to extract
);

HICON ExtractIcon(
  HINSTANCE hInst,          // instance handle
  LPCTSTR lpszExeFileName,  // file name
  UINT nIconIndex           // icon index
);

HICON ExtractAssociatedIcon(
  HINSTANCE hInst,    // application instance handle
  LPTSTR lpIconPath,  // file name
  LPWORD lpiIcon      // icon index
);

	SHFILEINFO shfi;
	memset(&shfi,0,sizeof(shfi));
	SHGetFileInfo("foo.bmp", 
		FILE_ATTRIBUTE_NORMAL,
		&shfi, sizeof(shfi),
		SHGFI_ICON|SHGFI_USEFILEATTRIBUTES);


*/
