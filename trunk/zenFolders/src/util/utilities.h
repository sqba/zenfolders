
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

//inline void TESTHR( HRESULT _hr ) { if FAILED(_hr) throw(_hr); }

inline void TESTHR( HRESULT _hr );

#ifndef __TESTHR__
#define __TESTHR__
void TESTHR( HRESULT _hr )
{
	if FAILED(_hr) throw(_hr);
}
#endif //__TESTHR__
