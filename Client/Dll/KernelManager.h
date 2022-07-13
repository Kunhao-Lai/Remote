#pragma once
#include "Manager.h"
class CKernelManager :
	public CManager
{
public:
	CKernelManager(_CIOCPClient* IOCPClient);
	~CKernelManager();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength);
	void ShutdownSystem();
private:
	HANDLE  m_ThreadHandle[0x1000];
	int     m_ThreadHandleCount;
};
DWORD  WINAPI  RemoteMessageProcedure(LPVOID  ParameterData);
DWORD  WINAPI  ProcessManagerProcedure(LPVOID  ParameterData);
DWORD  WINAPI  CMDManagerProcedure(LPVOID  ParameterData);
DWORD  WINAPI  WindowManagerProcedure(LPVOID  ParameterData);
DWORD  WINAPI  RemoteControlProcedure(LPVOID  ParameterData);
DWORD  WINAPI  FileManagerProcedure(LPVOID  ParameterData);
DWORD  WINAPI  AudioManagerProcedure(LPVOID  ParameterData);
DWORD  WINAPI  ServiceManagerProcedure(LPVOID  ParameterData);
DWORD  WINAPI  RegisterManagerProcedure(LPVOID  ParameterData);