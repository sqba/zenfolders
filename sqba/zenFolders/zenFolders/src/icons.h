#ifndef ICONS_H
#define ICONS_H


#include <windows.h>
#include <commctrl.h>


#define ICON_INDEX_FOLDER		1
#define ICON_INDEX_FOLDEROPEN	2
#define ICON_INDEX_FILE			3


struct tagFileType
{
//	TCHAR		szExtension[5];
	TCHAR		szPath[MAX_PATH];
	int			index;
	HICON		hIconSmall;
	HICON		hIconLarge;
	tagFileType *next;
	bool		bDontCache;
};


class CIcons  
{
public:
	CIcons(HINSTANCE);
	virtual ~CIcons();

	HICON GetIconLarge(UINT);
	HICON GetIconSmall(UINT);

	HIMAGELIST GetListLarge();
	HIMAGELIST GetListSmall();

	int GetIconIndex(LPCTSTR);
//	int GetIconIndex(LPITEMIDLIST);

private:
	bool		LoadShellIcons(HIMAGELIST, bool);
	HIMAGELIST	CreateImageList(int);
	int			AddAsociatedIcon(LPCTSTR);
	bool		AddShellIcon(HIMAGELIST, bool, int);
	HICON		GetAsociatedIcon(LPCTSTR, bool);
	HICON		ExtractShellIcon(int, bool);
	tagFileType *CreateNewFileType(LPCTSTR);
	HICON		LoadIcon(LPCTSTR, bool);

//	int			AddAsociatedIcon(LPCITEMIDLIST);
//	HICON		GetAsociatedIcon(LPCITEMIDLIST, bool);

//	HICON		ExtractIcon(LPCTSTR, int, bool);
//	void		AddIcon(HIMAGELIST, int, int);

private:
	HINSTANCE	m_hInst;
	HIMAGELIST	m_himlLarge;
	HIMAGELIST	m_himlSmall;
	HICON		m_ShellIcons[(ICON_INDEX_FILE+1)*2];
	tagFileType	*m_pFileTypes;
	int			m_iShellIconIndex;
};


typedef CIcons FAR *LPICONS;


#endif // ICONS_H
