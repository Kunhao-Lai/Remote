#pragma once
#include "Manager.h"
class CRemoteMessageManager :
	public CManager
{
public:
	CRemoteMessageManager(_CIOCPClient* IOCPClient);
	~CRemoteMessageManager();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength);
};
int  CALLBACK  DialogProcedure(HWND DlgHwnd, unsigned  int Message,
	WPARAM  wParam, LPARAM lParam);
void  OnInitDialog(HWND DlgHwnd);
void  OnTimerDialog(HWND DlgHwnd);
