#ifndef ICONS_H
#define ICONS_H


#include <windows.h>
#include <commctrl.h>


#define ICON_INDEX_FOLDER		1
#define ICON_INDEX_FOLDEROPEN	2
#define ICON_INDEX_FILE			3


typedef struct _FILETYPE
{
	TCHAR				szPath[MAX_PATH];
	int					index;
	HICON				hIconSmall;
	HICON				hIconLarge;
	struct _FILETYPE	*next;
} FILETYPE, FAR *LPFILETYPE;


class CIconList  
{
public:
	CIconList(HINSTANCE);
	virtual ~CIconList();

	HICON GetIconLarge(UINT);
	HICON GetIconSmall(UINT);

	HIMAGELIST GetListLarge();
	HIMAGELIST GetListSmall();

	int GetIconIndex(LPCTSTR);

private:
	bool		LoadShellIcons(HIMAGELIST, bool);
	HIMAGELIST	CreateImageList(int);
	int			AddAsociatedIcon(LPCTSTR);
	bool		AddShellIcon(HIMAGELIST, bool, int);
	HICON		GetAsociatedIcon(LPCTSTR, bool);
	HICON		ExtractShellIcon(int, bool);
	LPFILETYPE	CreateNewFileType(LPCTSTR);
	HICON		LoadIcon(LPCTSTR, bool);

private:
	HINSTANCE	m_hInst;
	HIMAGELIST	m_himlLarge;
	HIMAGELIST	m_himlSmall;
	HICON		m_ShellIcons[(ICON_INDEX_FILE+1)*2];
	LPFILETYPE	m_pFileTypes;
	int			m_iShellIconIndex;
};


//typedef CIconList FAR *LPICONLIST;


#endif // ICONS_H
