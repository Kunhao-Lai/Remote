#include "Manager.h"



CManager::CManager(_CIOCPClient*  IOCPClient)
{
	m_IOCPClient = IOCPClient;
	IOCPClient->SetManagerObject(this);
	m_EventDlgOpenHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
}


CManager::~CManager()
{
}
void  CManager::WaitingForServerDlgOpen()
{
	WaitForSingleObject(m_EventDlgOpenHandle, INFINITE);
	Sleep(150);
}
void  CManager::NotifyDialogIsOpen()
{
	SetEvent(m_EventDlgOpenHandle);
}
DWORD CManager::SetProcessPrivilege(HANDLE hProcess, bool bEnablePrivilege,LPCTSTR  RequireLevel)
{
	DWORD dwLastError;
	HANDLE hToken = 0;
	if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		dwLastError = GetLastError();
		if (hToken)
			CloseHandle(hToken);
		return dwLastError;
	}
	TOKEN_PRIVILEGES tokenPrivileges;
	memset(&tokenPrivileges, 0, sizeof(TOKEN_PRIVILEGES));
	LUID luid;
	if (!LookupPrivilegeValue(NULL, RequireLevel, &luid))
	{
		dwLastError = GetLastError();
		CloseHandle(hToken);
		return dwLastError;
	}
	tokenPrivileges.PrivilegeCount = 1;
	tokenPrivileges.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tokenPrivileges.Privileges[0].Attributes = 0;
	AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	dwLastError = GetLastError();
	CloseHandle(hToken);
	return dwLastError;
}
