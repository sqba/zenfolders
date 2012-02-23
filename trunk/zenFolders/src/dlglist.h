#ifndef DLGLIST_H
#define DLGLIST_H

#include <windows.h>
#include <shlobj.h>

#include "shlfldr.h"
#include "PidlMgr.h"

typedef struct _DIALOGLIST
{
	struct _DIALOGLIST		*pNext;
	CFolderPropertiesDlg	*pDialog;
} DIALOGLIST, FAR *LPDIALOGLIST;

class CDialogList  
{
public:
	CDialogList();
	virtual ~CDialogList();

	CFolderPropertiesDlg* GetNextDialog(CFolderPropertiesDlg*);
	bool AddToList(CFolderPropertiesDlg*);
	void RemoveFromList(CFolderPropertiesDlg*);
	
private:
	void DeleteList();
	bool Exists(CFolderPropertiesDlg*);
	
private:
	LPMALLOC		m_pMalloc;
	LPDIALOGLIST	m_pFirst;
	LPDIALOGLIST	m_pLast;
};

typedef CDialogList FAR *LPDIALOGSLIST;

#endif   //VIEWLIST_H
