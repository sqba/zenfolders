#ifndef DLGLIST_H
#define DLGLIST_H

#include <windows.h>
#include <shlobj.h>

#include "shlfldr.h"
#include "PidlMgr.h"

typedef struct tagDIALOGLIST
{
	struct tagDIALOGLIST	*pNext;
	CFolderPropertiesDlg	*pDialog;
}DIALOGLIST, FAR *LPDIALOGLIST;

class CDialogList  
{
public:
	CDialogList();
	virtual ~CDialogList();

	CFolderPropertiesDlg* GetNextDialog(CFolderPropertiesDlg*);
	BOOL AddToList(CFolderPropertiesDlg*);
	VOID RemoveFromList(CFolderPropertiesDlg*);
	
private:
	BOOL DeleteList(VOID);
	
private:
	LPMALLOC		m_pMalloc;
	LPDIALOGLIST	m_pFirst;
	LPDIALOGLIST	m_pLast;
};

typedef CDialogList FAR *LPDIALOGSLIST;

#endif   //VIEWLIST_H
