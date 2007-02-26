#ifndef SHELLEXTINIT_H
#define SHELLEXTINIT_H

#include <shlobj.h>

class CShellExtInit : public IShellExtInit  
{
public:
	CShellExtInit();
	virtual ~CShellExtInit();

public:
	//IShellExtInit functions
	STDMETHOD (Initialize) (ITEMIDLIST*, IDataObject*, HKEY);
};

#endif   //SHELLEXTINIT_H
