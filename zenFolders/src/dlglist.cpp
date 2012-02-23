// dlglist.cpp: implementation of the CDialogList class.
//
//////////////////////////////////////////////////////////////////////

#include "dlglist.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDialogList::CDialogList()
{
	m_pFirst = m_pLast = NULL;
	
	//get the shell's IMalloc pointer
	//we'll keep this until we get destroyed
	if(FAILED(::SHGetMalloc(&m_pMalloc)))
	{
		delete this;
		return;
	}
}

CDialogList::~CDialogList()
{
	DeleteList();
	
	if(m_pMalloc)
		m_pMalloc->Release();
}

CFolderPropertiesDlg* CDialogList::GetNextDialog(CFolderPropertiesDlg *pDialogIn)
{
	LPDIALOGLIST  pTemp;
	CFolderPropertiesDlg  *pDialogOut = NULL;
	
	if(!pDialogIn)
		pTemp = m_pFirst;
	else
	{
		for(pTemp = m_pFirst; pTemp; pTemp = pTemp->pNext)
		{
			if(pTemp->pDialog == pDialogIn)
			{
				pTemp = pTemp->pNext;
				break;
			}
		}
	}
	
	if(pTemp)
	{
		pDialogOut = pTemp->pDialog;
	}
	
	return pDialogOut;
}

void CDialogList::RemoveFromList(CFolderPropertiesDlg *pDialog)
{
	LPDIALOGLIST  pDelete;
	LPDIALOGLIST  pPrev;
	
	//find the item to be removed
	for(pDelete = m_pFirst, pPrev = NULL; pDelete; pDelete = pDelete->pNext)
	{
		if(pDelete->pDialog == pDialog)
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

void CDialogList::DeleteList()
{
	LPDIALOGLIST  pDelete;
	
	while(m_pFirst)
	{
		pDelete = m_pFirst;
		m_pFirst = pDelete->pNext;
		
		//free the list item
		m_pMalloc->Free(pDelete);
	}
	
	m_pFirst = m_pLast = NULL;
}

bool CDialogList::AddToList(CFolderPropertiesDlg *pDialog)
{
	if( Exists(pDialog) )
		return false;

	LPDIALOGLIST  pNew;
	
	pNew = (LPDIALOGLIST)m_pMalloc->Alloc(sizeof(DIALOGLIST));
	
	if(pNew)
	{
		//set the next pointer
		pNew->pNext = NULL;
		pNew->pDialog = pDialog;
		
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
		
		return true;
	}
	
	return false;
}

bool CDialogList::Exists(CFolderPropertiesDlg *pDialog)
{
	LPDIALOGLIST  pTemp;
	for(pTemp = m_pFirst; pTemp; pTemp = pTemp->pNext)
	{
		if( pTemp->pDialog->Equals(pDialog) )
		{
			pTemp->pDialog->BringToFront();
			return true;
		}
		//pTemp = pTemp->pNext;
	}
	return false;
}
