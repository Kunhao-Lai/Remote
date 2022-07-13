#pragma once
#include "Manager.h"
#include<TlHelp32.h>
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")

//进程信息
#pragma pack(1)
typedef struct _PROCESS_ITEM_INFORMATION_
{
	HANDLE ProcessID;
	char   ProcessImageName[MAX_PATH];
	char   ProcessFullPath[MAX_PATH];
	char   IsWow64[20];
}PROCESS_ITEM_INFORMATION, *PPROCESS_ITEM_INFORMATION;
typedef BOOL(__stdcall* LPFN_ISWOW64PROCESS)(HANDLE ProcessHandle, BOOL* IsResult);

typedef LONG KPRIORITY, *PKPRIORITY;
typedef struct _THREAD_ITEM_INFORMATION_
{
	LONG        BasePriority;
	ULONG       ContextSwitches;
	KPRIORITY   Priority;
	HANDLE      ThreadID;
	ULONG       ThreadState;
	ULONG       WaitReason;
	ULONG       WaitTime;
	PVOID       Teb;
	PVOID       ThreadStartAddress;
	HANDLE      ThreadHandle;
}THREAD_ITEM_INFORMATION, *PTHREAD_ITEM_INFORMATION;
typedef
BOOL(*LPFN_SEENUMTHREADINFO)(HANDLE ProcessID, vector<THREAD_ITEM_INFORMATION>& ThreadItemInfoVector);
typedef struct _HANDLE_ITEM_INFORMATION_
{
	USHORT HandleValue;
	PVOID  Object;
	string HandleName;
	string HandleType;
}HANDLE_ITEM_INFORMATION, *PHANDLE_ITEM_INFORMATION;
typedef
BOOL(*PFN_SEENUMHANDLEINFOBYPROCESSID)(HANDLE ProcessID, vector<HANDLE_ITEM_INFORMATION>& HandleItemInfoVector);

class CProcessManager :
	public CManager
{
public:
	CProcessManager(_CIOCPClient* IOCPClient);
	~CProcessManager();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength);
	BOOL  SendClientProcessList();
	BOOL SeEnumProcessByToolHelp32(vector<PROCESS_ITEM_INFORMATION>& ProcessItemInfoVector);
	BOOL SeIsWow64Process(HANDLE ProcessHandle, BOOL* IsResult);
	BOOL SeIsValidWritePoint(LPVOID VirtualAddress);
	//通过进程ID结束进程
	BOOL TerminateProcessFromId(HANDLE ProcessID);
	//发送线程信息
	BOOL SendProcessThreadInfoList(HANDLE ProcessID);
	//发送句柄信息SendProcessHandleInfoList
	BOOL SendProcessHandleInfoList(HANDLE ProcessID);
private:
	LPFN_ISWOW64PROCESS m_IsWow64Process = NULL;
	HANDLE    m_ProcessID = NULL;

};

