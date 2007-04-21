#ifndef ICONS_H
#define ICONS_H


#include <windows.h>
#include <commctrl.h>


#define ICON_INDEX_FOLDER		0
#define ICON_INDEX_FOLDEROPEN	1
#define ICON_INDEX_FILE			2


struct tagFileType
{
	TCHAR		szExtension[5];
	int			index;
	HICON		hIconSmall;
	HICON		hIconLarge;
	tagFileType *next;
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

private:
	bool		LoadShellIcons(HIMAGELIST, bool);
	HIMAGELIST	CreateImageList(int);
	int			AddAsociatedIcon(LPCTSTR);
	bool		AddShellIcon(HIMAGELIST, bool, int);
	HICON		GetAsociatedIcon(LPCTSTR, bool);
	HICON		ExtractShellIcon(int, bool);
	tagFileType *CreateNewFileType(LPCTSTR);
//	void		AddIcon(HIMAGELIST, int, int);

private:
	HINSTANCE	m_hInst;
	HIMAGELIST	m_himlLarge;
	HIMAGELIST	m_himlSmall;
	HICON		m_ShellIcons[6];
	tagFileType	*m_pFileTypes;
	int			m_iShellIconIndex;
};


typedef CIcons FAR *LPICONS;


#endif // ICONS_H
