#pragma once
#include "Manager.h"
class CServiceManager :
	public CManager
{
public:
	CServiceManager(_CIOCPClient* IOCPClient);
	~CServiceManager();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength);
	void   SendClientServiceList();
	LPBYTE  GetClientServiceList();
	void   ConfigClientService(PBYTE  BufferData, ULONG  BufferLength);
private:
	SC_HANDLE  m_ServiceManagerHandle;
};

