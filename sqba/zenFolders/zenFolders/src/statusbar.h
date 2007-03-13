#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <shlobj.h>
#include "shlfldr.h"

class CStatusBar  
{
public:
	static void Init(LPSHELLBROWSER);
	static void Fill(LPSHELLBROWSER, CShellFolder*);

private:
	static void SetText(int, LPCTSTR, LPSHELLBROWSER);
};

#endif   //STATUSBAR_H
