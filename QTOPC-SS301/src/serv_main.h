inline void cleanup_all(DWORD objid)
{	// Informs OLE that a class object, previously registered is no longer available for use
	if (FAILED(CoRevokeClassObject(objid)))  {};
	//destroyDriver();					// close port and destroy driver
	CoUninitialize();					// Closes the COM library on the current thread
}
