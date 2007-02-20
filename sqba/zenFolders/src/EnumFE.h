#ifndef ENUMFE_H
#define ENUMFE_H

#include <windows.h>

class CEnumFormatEtc : public IEnumFORMATETC
{
public:
	CEnumFormatEtc(FORMATETC*, UINT);
	~CEnumFormatEtc();

public:
	//IUnknown methods
	STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();

	// IEnumFORMATETC members
	STDMETHODIMP Next(ULONG, LPFORMATETC, ULONG*);
	STDMETHODIMP Skip(ULONG);
	STDMETHODIMP Reset(void);
	STDMETHODIMP Clone(LPENUMFORMATETC*);

private:
	DWORD		m_ObjRefCount;
	DWORD		m_cFormatEtc;
	DWORD		m_uCurrent;
	LPFORMATETC	m_pFormatEtc;
};

#endif// ENUMFE_H
