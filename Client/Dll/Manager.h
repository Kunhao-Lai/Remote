#pragma once
#include"_IOCPClient.h"
#include"Common.h"
#include<vector>
class _CIOCPClient;
class CManager
{
public:
	CManager(_CIOCPClient*  ClientObject);
	~CManager();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength) {}
	void  WaitingForServerDlgOpen();
	void  NotifyDialogIsOpen();
	DWORD SetProcessPrivilege(HANDLE hProcess, bool bEnablePrivilege, LPCTSTR  RequireLevel);
protected:
	//通信引擎类指针
	_CIOCPClient*  m_IOCPClient;
	HANDLE   m_EventDlgOpenHandle;
};

