#ifndef DATAOBJ_H
#define DATAOBJ_H

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>

#include "shlfldr.h"
#include "pidlmgr.h"
#include "resource.h"

#define SETDefFormatEtc(fe, cf, med) \
	{\
	(fe).cfFormat=cf; \
	(fe).dwAspect=DVASPECT_CONTENT; \
	(fe).ptd=NULL;\
	(fe).tymed=med;\
	(fe).lindex=-1;\
	};

class CDataObject : public IDataObject  
{
public:
	CDataObject(CShellFolder*, LPCITEMIDLIST*, UINT);
	virtual ~CDataObject();

public:
	//IUnknown methods
	STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();

	//IDataObject methods
	STDMETHODIMP GetData(LPFORMATETC, LPSTGMEDIUM);
	STDMETHODIMP GetDataHere(LPFORMATETC, LPSTGMEDIUM);
	STDMETHODIMP QueryGetData(LPFORMATETC);
	STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC);
	STDMETHODIMP SetData(LPFORMATETC, LPSTGMEDIUM, BOOL);
	STDMETHODIMP EnumFormatEtc(DWORD, IEnumFORMATETC**);
	STDMETHODIMP DAdvise(LPFORMATETC, DWORD, IAdviseSink*, LPDWORD);
	STDMETHODIMP DUnadvise(DWORD dwConnection);
	STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);

private:
	BOOL AllocPidlTable(DWORD);
	void FreePidlTable();
	BOOL FillPidlTable(LPCITEMIDLIST*, UINT);

private:
	DWORD          m_ObjRefCount;
	IMalloc        *m_pMalloc;
	LPITEMIDLIST   *m_aPidls;
	CShellFolder   *m_psfParent;
	UINT           m_uItemCount;
	ULONG		   m_cFormatEtc;
	LPFORMATETC    m_pFormatEtc;
	UINT           m_cfPrivateData;
	UINT           m_cfShellIDList;
	UINT           m_cfInShellDragLoop;
};

#endif// DATAOBJ_H
