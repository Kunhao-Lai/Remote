#pragma once
#include "Manager.h"
class CCmdManager :
	public CManager
{
public:
	CCmdManager(_CIOCPClient* IOCPClient);
	~CCmdManager();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength);
	static  DWORD  WINAPI  ReceiveProcedure(LPVOID  ParameterData);
public:
	HANDLE  m_ThreadHandle;
	BOOL    m_IsLoop;
	HANDLE  m_ReadHandle1;
	HANDLE  m_ReadHandle2;
	HANDLE  m_WriteHandle1;
	HANDLE  m_WriteHandle2;
	HANDLE  m_CmdProcessHandle;
	HANDLE  m_CmdThreadHandle;
	ULONG   m_ReceiveLength;

};

