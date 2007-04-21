#include "listview.h"


#define ITEM_BACKGROUND		RGB(240, 240, 240)


CListView::CListView(HINSTANCE hInst, HWND hWnd)
{
	m_iColumns = 0;
	m_lStyle = LVS_REPORT2;

	DWORD dwStyle;
	
	dwStyle = WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_SHAREIMAGELISTS | LVS_EDITLABELS | LVS_AUTOARRANGE;

	dwStyle |= LVS_REPORT;

	if(m_lStyle == LVS_REPORT)
		dwStyle |= LVS_NOSORTHEADER;
	
	m_hwndList = ::CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WC_LISTVIEW,
		NULL,
		dwStyle,
		0,
		0,
		0,
		0,
		hWnd,
		NULL, //(HMENU)ID_LISTVIEW,
		hInst,
		NULL);
	
	if(!m_hwndList)
	{
		delete this;
		return;
	}

	HWND hwndHeader = ListView_GetHeader(m_hwndList);
	LONG style = ::GetWindowLong(hwndHeader, GWL_STYLE);
	style |= HDS_BUTTONS;
	::SetWindowLong(hwndHeader, GWL_STYLE, style);

	//dwStyle = ListView_GetExtendedListViewStyle(m_hwndList);
	//dwStyle |= LVS_EX_FULLROWSELECT;
	//ListView_SetExtendedListViewStyle(m_hwndList, dwStyle);
}

CListView::~CListView()
{

}

void CListView::Clear()
{
	ListView_DeleteAllItems(m_hwndList);
}

UINT CListView::GetItemCount()
{
	return ListView_GetItemCount(m_hwndList);
}

UINT CListView::GetSelectedCount()
{
	return ListView_GetSelectedCount(m_hwndList);
}

BOOL CListView::GetItem(LVITEM *lvItem)
{
	return ListView_GetItem(m_hwndList, lvItem);
}

BOOL CListView::SortItems(PFNLVCOMPARE pfnCompare, LPARAM lParamSort)
{
	return ListView_SortItems(m_hwndList, pfnCompare, lParamSort);
}

UINT CListView::GetItemState(int i, UINT mask)
{
	return ListView_GetItemState(m_hwndList, i, mask);
}

BOOL CListView::Move(int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	if(m_hwndList)
	{
		return ::MoveWindow(m_hwndList, X, Y, nWidth, nHeight, bRepaint);
	}
	return FALSE;
}

int CListView::GetNextItem(int iStart, UINT flags)
{
	return ListView_GetNextItem(m_hwndList, iStart, flags);
}

int CListView::GetColumnWidth(int iCol)
{
	return ListView_GetColumnWidth(m_hwndList, iCol);
}

int CListView::InsertColumn(LPTSTR lpszName, int iWidth)
{
	LV_COLUMN lvColumn;
	
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.pszText = lpszName;
	lvColumn.cx = iWidth;

	return ListView_InsertColumn(m_hwndList, m_iColumns++, &lvColumn);
}

HIMAGELIST CListView::SetImageList(HIMAGELIST himl, int iImageList)
{
	return ListView_SetImageList(m_hwndList, himl, iImageList);
}

void CListView::SetRedraw(BOOL bRedraw)
{
	SendMessage(m_hwndList, WM_SETREDRAW, bRedraw, 0);
}

void CListView::Invalidate()
{
	InvalidateRect(m_hwndList, NULL, TRUE);
	UpdateWindow(m_hwndList);
}

BOOL CListView::Update(int iItem)
{
	return ListView_Update(m_hwndList, iItem);
}

HWND CListView::EditLabel(int iItem)
{
	return ListView_EditLabel(m_hwndList, iItem);
}

int CListView::InsertItem(LPARAM lParam)
{
	LV_ITEM  lvItem;
	
	ZeroMemory(&lvItem, sizeof(lvItem));
	
	//set the mask
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	
	//add the item to the end of the list
	lvItem.iItem = ListView_GetItemCount(m_hwndList);
	
	//set the item's data
	lvItem.lParam = lParam;
	
	//get text on a callback basis
	lvItem.pszText = LPSTR_TEXTCALLBACK;
	
	//get the image on a callback basis
	lvItem.iImage = I_IMAGECALLBACK;

	//add the item
	return ListView_InsertItem(m_hwndList, &lvItem);
}

void CListView::SetExtendedListViewStyle(DWORD dwExStyles)
{
	ListView_SetExtendedListViewStyle(m_hwndList, dwExStyles);
}

DWORD CListView::GetExtendedListViewStyle()
{
	return ListView_GetExtendedListViewStyle(m_hwndList);
}

void CListView::SelectItem(int index)
{
	ListView_SetItemState(
		m_hwndList,
		index,
		LVIS_SELECTED | LVIS_FOCUSED,
		LVIS_STATEIMAGEMASK);
}

void CListView::SetFocus()
{
	::SetFocus( m_hwndList );
}

BOOL CListView::Refresh()
{
	return ListView_RedrawItems(m_hwndList, 0, GetItemCount()-1);
}

LONG CListView::GetStyle()
{
	return m_lStyle;
/*
	LONG style = ::GetWindowLong(m_hwndList, GWL_STYLE);

	if(style & LVS_ICON)		return LVS_ICON;
	if(style & LVS_SMALLICON)	return LVS_SMALLICON;
	if(style & LVS_LIST)		return LVS_LIST;
	if(style & LVS_REPORT)		return LVS_REPORT;

	return 0;
*/
}

void CListView::SetStyle(LONG newStyle)
{
/*
	LONG style = ::GetWindowLong(m_hwndList, GWL_STYLE);

	if(style & LVS_ICON)		style ^= LVS_ICON;
	if(style & LVS_SMALLICON)	style ^= LVS_SMALLICON;
	if(style & LVS_LIST)		style ^= LVS_LIST;
	if(style & LVS_REPORT)		style ^= LVS_REPORT;

	style |= newStyle;
*/
	m_lStyle = newStyle;

	DWORD dwStyle;
	
	dwStyle = WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_SHAREIMAGELISTS | LVS_EDITLABELS | LVS_AUTOARRANGE;

	if(LVS_REPORT2 == m_lStyle)
	{
		dwStyle |= LVS_REPORT;

		DWORD dwExStyles = GetExtendedListViewStyle();
		if(!(dwExStyles & LVS_EX_FULLROWSELECT))
		{
			dwExStyles |= LVS_EX_FULLROWSELECT;
			SetExtendedListViewStyle( dwExStyles );
		}
	}
	else
	{
		dwStyle |= m_lStyle;

		if(LVS_REPORT == m_lStyle)
		{
			DWORD dwExStyles = GetExtendedListViewStyle();
			if(dwExStyles & LVS_EX_FULLROWSELECT)
			{
				dwExStyles ^= LVS_EX_FULLROWSELECT;
				SetExtendedListViewStyle( dwExStyles );
			}
		}
	}

	if((m_lStyle == LVS_REPORT) || (m_lStyle == LVS_REPORT2))
		dwStyle |= LVS_NOSORTHEADER;

	::SetWindowLong(m_hwndList, GWL_STYLE, dwStyle);

	if((m_lStyle == LVS_REPORT) || (m_lStyle == LVS_REPORT2))
		Refresh();
}

LRESULT CListView::OnCustomDraw(LPARAM lParam)
{
	int iRow;
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
	switch(lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		if(m_lStyle == LVS_REPORT2)
			return CDRF_NOTIFYITEMDRAW;
		else
			return CDRF_DODEFAULT;

	case CDDS_ITEMPREPAINT:
		iRow = (int)lplvcd->nmcd.dwItemSpec;
		if(iRow%2 == 0)
		{
			//lplvcd->clrText   = RGB(252, 177, 0);
			lplvcd->clrTextBk = ITEM_BACKGROUND;
			return CDRF_NEWFONT;
		}

	default:
		return CDRF_DODEFAULT;
	}
}
/*
LRESULT CListView::TableDraw(LPARAM lParam)
{
	int iRow;
	LPNMLVCUSTOMDRAW pListDraw = (LPNMLVCUSTOMDRAW)lParam;
	switch(pListDraw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
//		return (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW);
		//::SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (LONG_PTR)CDRF_NOTIFYITEMDRAW);
		//return TRUE; 
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
		iRow = (int)pListDraw->nmcd.dwItemSpec;
		if(iRow%2 == 0)
		{
			// pListDraw->clrText   = RGB(252, 177, 0);
			pListDraw->clrTextBk = RGB(202, 221,250);
			return CDRF_NEWFONT;
		}
	default:
		break;
	}
	return CDRF_DODEFAULT;
}
*/