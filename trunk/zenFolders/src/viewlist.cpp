#include "viewlist.h"


CViewList::CViewList()
{
	m_pFirst = m_pLast = NULL;
	
	//get the shell's IMalloc pointer
	//we'll keep this until we get destroyed
	if(FAILED(SHGetMalloc(&m_pMalloc)))
	{
		delete this;
		return;
	}
}

CViewList::~CViewList()
{
	DeleteList();
	
	if(m_pMalloc)
		m_pMalloc->Release();
}

CShellView* CViewList::GetNextView(CShellView *pViewIn)
{
	LPVIEWLIST  pTemp;
	CShellView  *pViewOut = NULL;
	
	if(!pViewIn)
		pTemp = m_pFirst;
	else
	{
		for(pTemp = m_pFirst; pTemp; pTemp = pTemp->pNext)
		{
			if(pTemp->pView == pViewIn)
			{
				pTemp = pTemp->pNext;
				break;
			}
		}
	}
	
	if(pTemp)
	{
		pViewOut = pTemp->pView;
	}
	
	return pViewOut;
}

BOOL CViewList::AddToList(CShellView *pView)
{
	LPVIEWLIST  pNew;
	
	pNew = (LPVIEWLIST)m_pMalloc->Alloc(sizeof(VIEWLIST));
	
	if(pNew)
	{
		//set the next pointer
		pNew->pNext = NULL;
		pNew->pView = pView;
		
		//is this the first item in the list?
		if(!m_pFirst)
		{
			m_pFirst = pNew;
		}
		
		if(m_pLast)
		{
			//add the new item to the end of the list
			m_pLast->pNext = pNew;
		}
		
		//update the last item pointer
		m_pLast = pNew;
		
		return TRUE;
	}
	
	return FALSE;
}

VOID CViewList::RemoveFromList(CShellView *pView)
{
	LPVIEWLIST  pDelete;
	LPVIEWLIST  pPrev;
	
	//find the item to be removed
	for(pDelete = m_pFirst, pPrev = NULL; pDelete; pDelete = pDelete->pNext)
	{
		if(pDelete->pView == pView)
			break;
		
		//save the previous item in the list
		pPrev = pDelete;
	}
	
	if(pDelete)
	{
		if(pPrev)
		{
			pPrev->pNext = pDelete->pNext;
		}
		else
		{
			//the item being deleted is the first item in the list
			m_pFirst = pDelete->pNext;
		}
		
		//was the last item deleted?
		if(!pDelete->pNext)
			m_pLast = pPrev;
		
		//free the list item
		m_pMalloc->Free(pDelete);
	}
}

BOOL CViewList::DeleteList(VOID)
{
	LPVIEWLIST  pDelete;
	
	while(m_pFirst)
	{
		pDelete = m_pFirst;
		m_pFirst = pDelete->pNext;
		
		//free the list item
		m_pMalloc->Free(pDelete);
	}
	
	m_pFirst = m_pLast = NULL;
	
	return TRUE;
}

void CViewList::Refresh()
{
	LPVIEWLIST pView = m_pFirst;
	while(pView)
	{
		pView->pView->Refresh();
		pView = pView->pNext;
	}
}

CShellView* CViewList::GetActiveView()
{
	if(m_pLast)
		return m_pLast->pView;
	else
		return NULL;
}
