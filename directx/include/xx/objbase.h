
/* Various object oriented things... I think :-) */

#ifndef _objbase_h
#define _objbase_h

/* IUnknown needs to be a typedef */
typedef void IUnknown;
#define LPUNKNOWN	void *

#define DEFINE_GUID(name, a, b, c, d, e, f, g, h, i, j, k) \
	static const GUID name = { a, b, c, { d, e, f, g, h, i, j, k } }

#if defined(__cplusplus) && !defined(CINTERFACE)

#define REFGUID			const GUID &
#define REFIID			REFGUID
#define REFCLSID		REFGUID
#define STDMETHOD(method)	virtual HRESULT __stdcall method
#define STDMETHOD_(type,method)	virtual type __stdcall method
#define PURE			= 0
#define THIS_
#define THIS			void
#define DECLARE_INTERFACE(iface) struct iface
#define DECLARE_INTERFACE_(iface, baseiface)	\
				struct iface : public baseiface

#else /* Not C++ */

#define REFGUID			const GUID * const
#define REFIID			REFGUID
#define REFCLSID		REFGUID
#define STDMETHOD(method)	HRESULT (__stdcall *method)
#define STDMETHOD_(type,method)	type (__stdcall *method)
#define PURE
#define THIS_			void *This,
#define THIS			void *This
#define DECLARE_INTERFACE(iface) \
				typedef struct iface##Vtbl iface##Vtbl; \
				typedef struct iface { \
					iface##Vtbl *lpVtbl; \
				} iface; \
				struct iface##Vtbl
#define DECLARE_INTERFACE_(iface, baseiface)	\
				DECLARE_INTERFACE(iface)

#endif /* C++ */

static inline int IsEqualGUID(GUID g1, GUID g2)
{
	return !memcmp(&g1, &g2, sizeof(GUID));
}

extern WINAPI HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
				DWORD dwClsContext, REFIID riid, LPVOID *ppv); 
extern WINAPI HRESULT CoInitialize(LPVOID pvReserved);
extern WINAPI void CoUninitialize(void);

#endif /* _objbase_h */
