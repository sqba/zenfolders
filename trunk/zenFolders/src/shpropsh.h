#ifndef SHPROPSH_H
#define SHPROPSH_H

#include <shlobj.h>

class CShellPropSheetExt : public IShellPropSheetExt  
{
public:
	CShellPropSheetExt();
	virtual ~CShellPropSheetExt();

public:
	//IShellPropSheetExt functions
	STDMETHOD (AddPages) (LPFNADDPROPSHEETPAGE, LPARAM);
	STDMETHOD (ReplacePage) (UINT, LPFNADDPROPSHEETPAGE, LPARAM);
};

#endif   //SHPROPSH_H
