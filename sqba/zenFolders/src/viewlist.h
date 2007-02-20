#ifndef VIEWLIST_H
#define VIEWLIST_H

#include <windows.h>
#include <shlobj.h>

#include "PidlMgr.h"
#include "ShlView.h"

typedef struct tagVIEWLIST
{
	struct tagVIEWLIST   *pNext;
	CShellView           *pView;
}VIEWLIST, FAR *LPVIEWLIST;

class CViewList
{
public:
	CViewList();
	~CViewList();
	
	CShellView* GetNextView(CShellView*);
	CShellView* GetActiveView();
	BOOL AddToList(CShellView*);
	VOID RemoveFromList(CShellView*);

	void Refresh();
	
private:
	BOOL DeleteList(VOID);
	
private:
	LPMALLOC	m_pMalloc;
	LPVIEWLIST	m_pFirst;
	LPVIEWLIST	m_pLast;
};

typedef CViewList FAR *LPVIEWSLIST;

#endif   //VIEWLIST_H
