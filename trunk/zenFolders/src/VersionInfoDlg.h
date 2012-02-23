// VersionInfoDlg.h: interface for the CVersionInfoDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VERSIONINFODLG_H__466E966C_16B7_4D90_B4C2_4C0878851004__INCLUDED_)
#define AFX_VERSIONINFODLG_H__466E966C_16B7_4D90_B4C2_4C0878851004__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include "shlfldr.h"

class CVersionInfoDlg  
{
public:
	CVersionInfoDlg(CShellFolder*);
	virtual ~CVersionInfoDlg();

	void Show();

private:
	static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

	void DisplayPath(HWND);
	void DisplayVersion(HWND);

private:
	CShellFolder	*m_pParent;
	HWND			m_hDlg;
};

#endif // !defined(AFX_VERSIONINFODLG_H__466E966C_16B7_4D90_B4C2_4C0878851004__INCLUDED_)
