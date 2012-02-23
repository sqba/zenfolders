#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <windows.h>
#include <commctrl.h>

#define LVS_REPORT2		0x8000

class CListView  
{
public:
	CListView(HINSTANCE, HWND);
	virtual ~CListView();

	void Clear();
	UINT GetItemCount();
	UINT GetSelectedCount();
	BOOL GetItem(LVITEM*);
	UINT GetItemState(int, UINT);
	BOOL SortItems(PFNLVCOMPARE, LPARAM);
	BOOL Move(int, int, int, int, BOOL);
	int GetNextItem(int, UINT);
	void UpdateShellSettings(void);
	int GetColumnWidth(int);
	int InsertColumn(LPTSTR, int);
	HIMAGELIST SetImageList(HIMAGELIST, int);
	void SetRedraw(BOOL);
	void Invalidate();
	BOOL Update(int);
	HWND EditLabel(int);
	int InsertItem(LPARAM);
	void SetExtendedListViewStyle(DWORD);
	void SelectItem(int);
	void SetFocus();
	void SetStyle(LONG);
	LONG GetStyle();
	LRESULT OnCustomDraw(LPARAM);
	DWORD GetExtendedListViewStyle();
	BOOL Refresh();

//	static LRESULT TableDraw(LPARAM);
//	HWND GetHwnd() { return m_hwndList; }

private:
	int		m_iColumns;
	LONG	m_lStyle;
	HWND	m_hwndList;
};

#endif   //LISTVIEW_H
