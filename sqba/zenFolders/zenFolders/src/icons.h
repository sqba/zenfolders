#ifndef ICONS_H
#define ICONS_H

#include <windows.h>
#include <commctrl.h>

#define ICON_INDEX_FOLDER		1
#define ICON_INDEX_FOLDEROPEN	2
#define ICON_INDEX_FILE			3

struct tagExtension
{
	TCHAR		szExtension[5];
	int			index;
	HICON		hIconSmall;
	HICON		hIconLarge;
	tagExtension *next;
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
	void AddDefaultIcons(HIMAGELIST, bool);
	HIMAGELIST CreateImageList(int);
	int AddIcon(LPCTSTR);
	void AddIcon(HIMAGELIST, int, int);
	void AddShellIcon(HIMAGELIST, int, bool);
	HICON GetAsociatedIcon(LPCTSTR, bool);
	tagExtension *CreateNewExtension(LPCTSTR);
	HICON ExtractShellIcon(int, bool);

private:
	HINSTANCE	m_hInst;
	HIMAGELIST	m_himlLarge;
	HIMAGELIST	m_himlSmall;
	tagExtension	*m_pExtensions;
	HICON		m_SystemIcons[6];
};

typedef CIcons FAR *LPICONS;

#endif // ICONS_H
