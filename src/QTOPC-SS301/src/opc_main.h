
#include <unknwn.h>
#include "../3rd/opcda/opcda.h"
#include "../3rd/lightopc/lightopc.h"

class myClassFactory: public IClassFactory
{
public:
	LONG RefCount;			// reference counter
	LONG server_count;		// server counter
	CRITICAL_SECTION lk;	// protect RefCount

	// when creating interface zero all counter
	myClassFactory(): RefCount(0), server_count(0)
	{ InitializeCriticalSection(&lk); }

	~myClassFactory()
	{ DeleteCriticalSection(&lk); }

	// IUnknown realisation
	STDMETHODIMP QueryInterface(REFIID, LPVOID*);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// IClassFactory realisation
	STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID*);
	STDMETHODIMP LockServer(BOOL);

	inline LONG getRefCount(void)
	{
		LONG rc;
		EnterCriticalSection(&lk);		// attempt recieve variable whom may be used by another threads
		rc = RefCount;								// rc = client counter
		LeaveCriticalSection(&lk);
		return rc;
	}

	inline int in_use(void)
	{
		int rv;
		//EnterCriticalSection(&lk);
		rv = RefCount | server_count;
		//LeaveCriticalSection(&lk);
		return rv;
	}
	inline void serverAdd(void)
	{
		InterlockedIncrement(&server_count);	// increment server counter
	}
	inline void serverRemove(void)
	{
		InterlockedDecrement(&server_count);	// decrement server counter
	}
};


//----- IUnknown -------------------------------------------------------------------------
STDMETHODIMP myClassFactory::QueryInterface(REFIID iid, LPVOID* ppInterface)
{
	if (ppInterface == NULL) return E_INVALIDARG;	// pointer to interface missed (NULL)

	if (iid == IID_IUnknown || iid == IID_IClassFactory)	// legal IID
	{
		//m_log.Write(L_DEBUG, "myClassFactory::QueryInterface() Ok");
		*ppInterface = this;		// interface succesfully returned
		AddRef();					// adding reference to interface
		return S_OK;				// return succesfully
	}
	//m_log.Write(L_DEBUG, "myClassFactory::QueryInterface() Failed");
	*ppInterface = NULL;			// no interface returned
	return E_NOINTERFACE;			// error = No Interface
}


STDMETHODIMP_(ULONG) myClassFactory::AddRef(void)	// new client was connected
{
	ULONG rv;
	EnterCriticalSection(&lk);
	rv = (ULONG)++RefCount;							// increment counter of client
	LeaveCriticalSection(&lk);
	//m_log.Write(L_DEBUG, "myClassFactory::AddRef(%d)", rv);
	return rv;
}


STDMETHODIMP_(ULONG) myClassFactory::Release(void)	// client has been disconnected
{
	ULONG rv;
	EnterCriticalSection(&lk);
	rv = (ULONG)--RefCount;							// decrement client counter
	LeaveCriticalSection(&lk);
	//m_log.Write(L_DEBUG, "myClassFactory::Release(%d)", rv);
	return rv;
}


//----- IClassFactory ----------------------------------------------------------------------
STDMETHODIMP myClassFactory::LockServer(BOOL fLock)
{
	if (fLock)	AddRef();
	else		    Release();
	//m_log.Write(L_DEBUG, "myClassFactory::LockServer(%d)", fLock);
	return S_OK;
}


STDMETHODIMP myClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppvObject)
{
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION; // Aggregation is not supported by this code

	IUnknown *server = 0;
	AddRef(); // for a_server_finished()
	if (loClientCreate(my_service, (loClient**)&server, 0, &vendor, a_server_finished, this))
	{
		//m_log.Write(L_DEBUG, "myClassFactory::loClientCreate() failed");
		Release();
		return E_OUTOFMEMORY;
	}
	serverAdd();
	HRESULT hr = server->QueryInterface(riid, ppvObject);
	if (FAILED(hr))
		m_log.Write(L_DEBUG, "myClassFactory::loClient QueryInterface() failed");
	else
	{
		loSetState(my_service, (loClient*)server, loOP_OPERATE, OPCstatus, 0);
		m_log.Write(L_INFO, "Количество подключенных клиентов = %ld", server_count);
	}

	server->Release();
	return hr;
}


static myClassFactory my_CF;


static void a_server_finished(void *a, loService *b, loClient *c)
{
	//loSetState(my_service, 0, loOP_SHUTDOWN, OPC_STATUS_SUSPENDED, 0);
	my_CF.serverRemove();
	if (a) ((myClassFactory*)a)->Release();
	m_log.Write(L_INFO, "Завершена работа клиента #%lu", my_CF.server_count + 1);
}

