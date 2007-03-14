// icons.cpp: implementation of the CIcons class.
//
//////////////////////////////////////////////////////////////////////

#include "icons.h"
#include "util/string.h"
#include "resource.h"


#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


#define SI_UNKNOWN						0	//Unknown File Type
#define SI_DEF_DOCUMENT					1	//Default document
#define SI_DEF_APPLICATION				2	//Default application
#define SI_FOLDER_CLOSED				3	//Closed folder
#define SI_FOLDER_OPEN					4	//Open folder
#define SI_FLOPPY_514					5	//5 1/4 floppy
#define SI_FLOPPY_35					6	//3 1/2 floppy
#define SI_REMOVABLE					7	//Removable drive
#define SI_HDD							8	//Hard disk drive
#define SI_NETWORKDRIVE					9	//Network drive
#define SI_NETWORKDRIVE_DISCONNECTED	10	//network drive offline
#define SI_CDROM						11	//CD drive
#define SI_RAMDISK						12	//RAM disk
#define SI_NETWORK						13	//Entire network
//#define 14		?						
#define SI_MYCOMPUTER					15	//My Computer
#define SI_PRINTMANAGER					16	//Printer Manager
#define SI_NETWORK_NEIGHBORHOOD			17	//Network Neighborhood
#define SI_NETWORK_WORKGROUP			18	//Network Workgroup
#define SI_STARTMENU_PROGRAMS			19	//Start Menu Programs
#define SI_STARTMENU_DOCUMENTS			20	//Start Menu Documents
#define SI_STARTMENU_SETTINGS			21	//Start Menu Settings
#define SI_STARTMENU_FIND				22	//Start Menu Find
#define SI_STARTMENU_HELP				23	//Start Menu Help
#define SI_STARTMENU_RUN				24	//Start Menu Run
#define SI_STARTMENU_SUSPEND			25	//Start Menu Suspend
#define SI_STARTMENU_DOCKING			26	//Start Menu Docking
#define SI_STARTMENU_SHUTDOWN			27	//Start Menu Shutdown
#define SI_SHARE						28	//Sharing overlay (hand)
#define SI_SHORTCUT						29	//Shortcut overlay (small arrow)
#define SI_PRINTER_DEFAULT				30	//Default printer overlay (small tick)
#define SI_RECYCLEBIN_EMPTY				31	//Recycle bin empty
#define SI_RECYCLEBIN_FULL				32	//Recycle bin full
#define SI_DUN							33	//Dial-up Network Folder
#define SI_DESKTOP						34	//Desktop
#define SI_CONTROLPANEL					35	//Control Panel
#define SI_PROGRAMGROUPS				36	//Program Group
#define SI_PRINTER						37	//Printer
#define SI_FONT							38	//Font Folder
#define SI_TASKBAR						39	//Taskbar
#define SI_AUDIO_CD						40	//Audio CD
//#define 41		?						
//#define 42		?						
#define SI_FAVORITES					43	//IE favorites
#define SI_LOGOFF						44	//Start Menu Logoff
//#define 45		?						
//#define 46		?						
#define SI_LOCK							47	//Lock
#define SI_HIBERNATE					48	//Hibernate


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

	::SHGetFileInfo(
		pszPath, 
		FILE_ATTRIBUTE_NORMAL,
		&shfi,
		sizeof(shfi),
		dwAttributes);

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
    HICON hIcon = NULL;

    ::ExtractIconEx(
        TEXT("SHELL32.DLL"),
        nIndex,
		bSmall ? NULL : &hIcon,
        bSmall ? &hIcon : NULL,
        1);

    return hIcon;
}
