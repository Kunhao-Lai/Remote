#include "RemoteMessageManager.h"
#include"Common.h"
#include"resource.h"
#include"mmsystem.h"
#pragma comment(lib,"WINMM.LIB")

extern  HINSTANCE  __InstanceHandle;
CHAR  __BufferData[0x1000] = { 0 };
#define WINDOW_WIDTH  220
#define WINDOW_HEIGHT 150
#define ID_TIMER_POP_WINDOW 1
#define ID_TIMER_DELAY_DISPLAY 2
#define ID_TIMER_CLOSE_WINDOW 3
int __TimeEvent = 0;
_CIOCPClient*  __IOCPClient = NULL;
CRemoteMessageManager::CRemoteMessageManager(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	//回传数据包到服务器
	BYTE  IsToken = CLIENT_REMOTE_MESSAGE_REPLY;
	m_IOCPClient->OnSending((char*)&IsToken, 1);
	__IOCPClient = IOCPClient;
	WaitingForServerDlgOpen();
}


CRemoteMessageManager::~CRemoteMessageManager()
{
	printf("~CRemoteMessageManager()\r\n");
}
void  CRemoteMessageManager::HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength)
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
		//获得远程消息的数据内容
		memcpy(__BufferData, BufferData, BufferLength);
		//构建一个Dialog类
		DialogBoxA(__InstanceHandle, MAKEINTRESOURCE(IDD_DIALOG_REMOTE_MESSAGE_MANAGER),
			NULL, DialogProcedure);
		break;
	}
	}
}
int  CALLBACK  DialogProcedure(HWND DlgHwnd, unsigned  int Message,
	WPARAM  wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_TIMER:
	{
		OnTimerDialog(DlgHwnd);
		break;
	}
	case WM_INITDIALOG:
	{
		OnInitDialog(DlgHwnd);
		break;
	}
	}
	return 0;
}
void  OnInitDialog(HWND DlgHwnd)
{
	MoveWindow(DlgHwnd, 0, 0, 0, 0, TRUE);
	//在控件上设置数据
	SetDlgItemText(DlgHwnd, IDC_EDIT_DIALOG_REMOTE_MESSAGE, __BufferData);
	memset(__BufferData, 0, sizeof(__BufferData));

	__TimeEvent = ID_TIMER_POP_WINDOW;

	SetTimer(DlgHwnd, __TimeEvent, 1, NULL);

	PlaySound(MAKEINTRESOURCE(IDR_WAVE_REMOTE_MESSAGE),
		__InstanceHandle, SND_ASYNC | SND_RESOURCE | SND_NODEFAULT);
}
void  OnTimerDialog(HWND DlgHwnd)
{
	RECT  Rect;
	static  int  Height = 0;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &Rect, 0);
	int y = Rect.bottom - Rect.top;
	int x = 0;
	switch (__TimeEvent)
	{
	case ID_TIMER_POP_WINDOW:
	{
		if (Height <= WINDOW_HEIGHT)
		{
			Height += 3;
			MoveWindow(DlgHwnd, x, y - Height, WINDOW_WIDTH, Height, TRUE);
		}
		else
		{
			KillTimer(DlgHwnd, ID_TIMER_POP_WINDOW);
			__TimeEvent = ID_TIMER_DELAY_DISPLAY;
			SetTimer(DlgHwnd, __TimeEvent, 7000, NULL);
		}
		break;
	}
	case ID_TIMER_DELAY_DISPLAY:
	{
		KillTimer(DlgHwnd, ID_TIMER_DELAY_DISPLAY);
		__TimeEvent = ID_TIMER_CLOSE_WINDOW;
		SetTimer(DlgHwnd, __TimeEvent, 5, NULL);
		break;
	}
	case ID_TIMER_CLOSE_WINDOW:
	{
		if (Height >= 0)
		{
			Height -= 5;
			MoveWindow(DlgHwnd, x, y - Height, WINDOW_WIDTH, Height, TRUE);
		}
		else
		{
			KillTimer(DlgHwnd, ID_TIMER_CLOSE_WINDOW);
			//通知服务器可以发送新的消息
			BYTE IsToken = CLIENT_REMOTE_MESSAGE_COMPLETE;
			__IOCPClient->OnSending((char*)&IsToken, 1);
			EndDialog(DlgHwnd, 0);
		}
		break;
	}
	}
}

