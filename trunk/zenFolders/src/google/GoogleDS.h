#ifndef GOOGLEDS_H
#define GOOGLEDS_H

#include <windows.h>
#include <oleauto.h>
#include "../pidl.h"

#pragma warning(disable:4786)
#import "../../GoogleDesktopAPI2.dll"
using namespace GoogleDesktopAPILib;

#define RELEVANCE_RANKING	0
#define RECENCY_RANKING		1

class CGoogleDS  
{
public:
	static IGoogleDesktopQueryResultSet *Query(LPPIDLDATA, BOOL);

	static bool RegisterPlugin();
	static bool UnregisterPlugin();

	static bool IsInstalled();
	static bool IsRegistered();

private:
	static IGoogleDesktopQueryResultSet *Query(const OLECHAR*, const OLECHAR*, int, int, HRESULT*);
	static IGoogleDesktopQueryResultSet *QueryEx(const OLECHAR*, const OLECHAR*, int, int, HRESULT*);
	const TCHAR* GDErrorToString(HRESULT);

private:
	static HRESULT AddPairToSafeArray(int, const OLECHAR*, SAFEARRAY FAR*);
};

#endif   //GOOGLEDS_H
