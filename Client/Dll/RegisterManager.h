#pragma once
#include "Manager.h"
#include"Register.h"
class CRegisterManager :
	public CManager
{
public:
	CRegisterManager(_CIOCPClient* IOCPClient);
	~CRegisterManager();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength);
	void    FindRegisterData(char  IsToken, char*  FullPath);
};

