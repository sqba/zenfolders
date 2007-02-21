// Dialog.h: interface for the CDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIALOG_H__564EC7EA_AAAF_486F_9B1B_6FAF1642CE15__INCLUDED_)
#define AFX_DIALOG_H__564EC7EA_AAAF_486F_9B1B_6FAF1642CE15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

class CDialog  
{
public:
	CDialog(HINSTANCE, int);
	virtual ~CDialog();

	void SetMessage(LPCTSTR);
	void HideOkButton();
	void SetCancelButtonText(LPCTSTR);

	BOOL OnInit();
	BOOL OnClose();
	BOOL OnCommand(int, int);

public:
	HWND m_hwnd;
};

#endif // !defined(AFX_DIALOG_H__564EC7EA_AAAF_486F_9B1B_6FAF1642CE15__INCLUDED_)
