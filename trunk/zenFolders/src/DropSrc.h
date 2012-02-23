#ifndef DROPSRC_H
#define DROPSRC_H

#include <windows.h>

class CDropSource : public IDropSource
{
public: 
	// Contstructor and Destructor
	CDropSource();
	~CDropSource();

public:
	// IUnknown Interface members
	STDMETHODIMP QueryInterface(REFIID, LPVOID*);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// IDropSource Interface members
	STDMETHODIMP QueryContinueDrag(BOOL, DWORD);
	STDMETHODIMP GiveFeedback(DWORD);

private: 
	DWORD m_ObjRefCount;
};

#endif   //DROPSRC_H
