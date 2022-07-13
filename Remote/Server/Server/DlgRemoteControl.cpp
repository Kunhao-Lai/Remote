// DlgRemoteControl.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRemoteControl.h"
#include "afxdialogex.h"
#include"Common.h"
#include"resource.h"


enum
{
	IDM_REMOTE_CONTROL = 0X1010,
	IDM_REMOTE_CONTROL_SEND_CTRL_ALT_DEL,
	IDM_REMOTE_CONTROL_TRACE_CURSOR,                //跟踪显示远程鼠标
	IDM_REMOTE_CONTROL_BLOCK_INPUT,                 //锁定远程计算机输入
	IDM_REMOTE_CONTROL_SAVE_DIB,                    //保存图片
	IDM_REMOTE_CONTROL_GET_CLIPBOARD,               //获取剪贴板
	IDM_REMOTE_CONTROL_SET_CLIPBOARD,               //设置剪贴板
};
// CDlgRemoteControl 对话框

IMPLEMENT_DYNAMIC(CDlgRemoteControl, CDialog)

CDlgRemoteControl::CDlgRemoteControl(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialog(IDD_DIALOG_REMOTE_CONTROL, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_HScrollPosition = 0;
	m_VScrollPosition = 0;
	m_IsTraceCursor = FALSE;
	m_IsControl = FALSE;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	if (m_ContextObject == NULL)
	{
		return;
	}
	ULONG  BufferLength = m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1;
	m_BitmapInfo = (BITMAPINFO*) new BYTE[BufferLength];
	if (m_BitmapInfo == NULL)
	{
		return;
	}
	//拷贝位图头信息
	memcpy(m_BitmapInfo, m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1), BufferLength);
	m_hAccel_TraceCursor = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_REMOTE_CONTROL));
	m_hAccel_ControlScreen = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_REMOTE_CONTROL));
	m_hAccel_SaveSnapshot = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_REMOTE_CONTROL));
	m_hAccel_BlockInput = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_REMOTE_CONTROL));
	m_hAccel_GetClipBoard = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_REMOTE_CONTROL));
	m_hAccel_SetClipBoard = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_REMOTE_CONTROL));
}

CDlgRemoteControl::~CDlgRemoteControl()
{
	::ReleaseDC(m_hWnd, m_DesktopDCHandle);
	//回收工具箱
	if (m_DesktopMemoryDCHandle != NULL)
	{
		DeleteDC(m_DesktopMemoryDCHandle);
		DeleteObject(m_BitmapHandle);
		if (m_BitmapData != NULL)
		{
			m_BitmapData = NULL;
		}
		m_DesktopMemoryDCHandle = NULL;
	}
//	::ReleaseDC(m_hWnd, m_DesktopDCHandle);
//	::DeleteDC(m_DesktopMemoryDCHandle);
//	::DeleteObject(m_BitmapHandle);
	if (m_BitmapInfo != NULL)
	{
		delete[] m_BitmapInfo;
		m_BitmapInfo = NULL;
	}
}

void CDlgRemoteControl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgRemoteControl, CDialog)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SYSCOMMAND()
	ON_COMMAND(ID_ACCELERATOR_TRACE_CURSOR, &CDlgRemoteControl::OnAcceleratorTraceCursor)
	ON_COMMAND(ID_ACCELERATOR_CONTROL_SCREEN, &CDlgRemoteControl::OnAcceleratorControlScreen)
	ON_COMMAND(ID_ACCELERATOR_SAVE_DIB, &CDlgRemoteControl::OnAcceleratorSaveDib)
	ON_COMMAND(ID_ACCELERATOR_BLOCK_INPUT, &CDlgRemoteControl::OnAcceleratorBlockInput)
	ON_COMMAND(ID_ACCELERATOR_GET_CLIPBOARD, &CDlgRemoteControl::OnAcceleratorGetClipboard)
	ON_COMMAND(ID_ACCELERATOR_SET_CLIPBOARD, &CDlgRemoteControl::OnAcceleratorSetClipboard)
END_MESSAGE_MAP()


// CDlgRemoteControl 消息处理程序


BOOL CDlgRemoteControl::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetIcon(m_hIcon, FALSE);

	//将客户端IP显示在标题栏
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int  ClientAddressLength = sizeof(ClientAddress);
	//得到连接的IP地址
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);
	v1.Format("\\\\%s - 远程控制", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	m_DesktopDCHandle = ::GetDC(m_hWnd);
	m_DesktopMemoryDCHandle = CreateCompatibleDC(m_DesktopDCHandle);
	m_BitmapHandle = CreateDIBSection(m_DesktopDCHandle, m_BitmapInfo,
		DIB_RGB_COLORS, &m_BitmapData, NULL, NULL);
	//初始化工具箱
	SelectObject(m_DesktopMemoryDCHandle, m_BitmapHandle);

	SetWindowPos(NULL, 0, 0, m_BitmapInfo->bmiHeader.biWidth / 2, m_BitmapInfo->bmiHeader.biHeight / 2, 0);

	SetScrollRange(SB_HORZ, 0, m_BitmapInfo->bmiHeader.biWidth / 1.8);
	SetScrollRange(SB_VERT, 0, m_BitmapInfo->bmiHeader.biHeight / 1.8);

	//获得系统菜单
	CMenu*  SysMenu = GetSystemMenu(FALSE);
	if (SysMenu != NULL)
	{
		SysMenu->AppendMenu(MF_SEPARATOR);
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL,"控制屏幕(&Y)");               //快捷键
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL_TRACE_CURSOR, "跟踪被控端鼠标(&T)");
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL_BLOCK_INPUT, "锁定被控端鼠标和键盘(&L)");
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL_SAVE_DIB, "保存快照(&S)");
		SysMenu->AppendMenu(MF_SEPARATOR);
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL_GET_CLIPBOARD, "获取剪贴板(&R)");
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL_SET_CLIPBOARD, "设置剪贴板(&F)");
		SysMenu->AppendMenu(MF_SEPARATOR);
	}
	//客户端屏幕中的鼠标位置
	m_ClientCursorPosition.x = 0;
	m_ClientCursorPosition.y = 0;
	//回传数据
	BYTE  IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgRemoteControl::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_ContextObject != NULL)
	{
		m_ContextObject->DialogHandle = NULL;
		m_ContextObject->DialogID = 0;
		CancelIo((HANDLE)m_ContextObject->ClientSocket);
		closesocket(m_ContextObject->ClientSocket);
	}
	CDialog::OnClose();
}
void  CDlgRemoteControl::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray()[0])
	{
	case  CLIENT_REMOTE_CONTROL_FIRST_SCREEN:
	{
		DrawFirstScreen();
		break;
	}
	case  CLIENT_REMOTE_CONTROL_NEXT_SCREEN:
	{
#define  ALGORITHM_DIFF  1
		if (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[1] == ALGORITHM_DIFF)
		{
			DrawNextScreen();
		}
		break;
	}
	case  CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REPLY:
	{
		//将接受到的客户端数据放入到主控端的剪切板
		UpdateClipboard((char*)m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1),
			m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1);

		break;
	}
	}

}
void  CDlgRemoteControl::DrawFirstScreen()
{
	memcpy(m_BitmapData, m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1),
		m_BitmapInfo->bmiHeader.biSizeImage);
	PostMessage(WM_PAINT);
}
void  CDlgRemoteControl::DrawNextScreen()
{
	BOOL  IsChange = FALSE;
	//标识+算法+光标位置+光标类型
	ULONG  v1 = 1 + 1 + sizeof(POINT) + sizeof(BYTE);
	LPVOID  PreScreenData = m_BitmapData;
	LPVOID  CurrentBufferData = m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(v1);
	ULONG   CurrentBufferLength = m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - v1;
	POINT   PreClientCursorPos;
	memcpy(&PreClientCursorPos, &m_ClientCursorPosition, sizeof(POINT));
	memcpy(&m_ClientCursorPosition, m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(2), sizeof(POINT));
	//鼠标移动了
	if (memcmp(&PreClientCursorPos, &m_ClientCursorPosition, sizeof(POINT)) != 0)
	{
		IsChange = TRUE;
	}
	//屏幕是否有变化
	if (CurrentBufferLength > 0)
	{
		IsChange = TRUE;
	}
	CopyScreenData(PreScreenData, CurrentBufferData, CurrentBufferLength);
	if (IsChange)
	{
		PostMessage(WM_PAINT);
	}
}
void CDlgRemoteControl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialog::OnPaint()
	BitBlt(m_DesktopDCHandle, 0, 0,
		m_BitmapInfo->bmiHeader.biWidth,
		m_BitmapInfo->bmiHeader.biHeight,
		m_DesktopMemoryDCHandle,
		m_HScrollPosition,
		m_VScrollPosition,
		SRCCOPY);
	if (m_IsTraceCursor)
	{
		DrawIconEx(
			m_DesktopDCHandle,
			m_ClientCursorPosition.x - m_HScrollPosition,
			m_ClientCursorPosition.y - m_VScrollPosition,
			m_hIcon,
			0, 0, 0,
			NULL,
			DI_NORMAL | DI_COMPAT);
	}
}


void CDlgRemoteControl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO  ScrollInfo;
	int i;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_ALL;
	GetScrollInfo(SB_VERT, &ScrollInfo);
	switch (nSBCode)
	{
	case  SB_LINEUP:
	{
		i = nPos - 1;
		break;
	}
	case  SB_LINEDOWN:
	{
		i = nPos + 1;
		break;
	}
	case  SB_THUMBPOSITION:
	case  SB_THUMBTRACK:
	{
		i = ScrollInfo.nTrackPos;
		break;
	}
	default:
		return;
	}
	i = max(i, ScrollInfo.nMin);
	i = min(i, (int)(ScrollInfo.nMax - ScrollInfo.nPage + 1));
	RECT  Rect;
	GetClientRect(&Rect);
	if ((Rect.bottom + i) > m_BitmapInfo->bmiHeader.biHeight)
	{
		i = m_BitmapInfo->bmiHeader.biHeight - Rect.bottom;
	}
	InterlockedExchange((PLONG)&m_VScrollPosition, i);
	SetScrollPos(SB_VERT, i);
	OnPaint();
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CDlgRemoteControl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO  ScrollInfo;
	int i;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_ALL;
	GetScrollInfo(SB_HORZ, &ScrollInfo);
	switch (nSBCode)
	{
	case  SB_LINEUP:
	{
		i = nPos - 1;
		break;
	}
	case  SB_LINEDOWN:
	{
		i = nPos + 1;
		break;
	}
	case  SB_THUMBPOSITION:
	case  SB_THUMBTRACK:
	{
		i = ScrollInfo.nTrackPos;
		break;
	}
	default:
		return;
	}
	i = max(i, ScrollInfo.nMin);
	i = min(i, (int)(ScrollInfo.nMax - ScrollInfo.nPage + 1));
	RECT  Rect;
	GetClientRect(&Rect);
	if ((Rect.right + i) > m_BitmapInfo->bmiHeader.biWidth)
	{
		i = m_BitmapInfo->bmiHeader.biWidth - Rect.right;
	}
	InterlockedExchange((PLONG)&m_HScrollPosition, i);
	SetScrollPos(SB_HORZ, i);
	OnPaint();
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

//该消息接收包括系统菜单等待窗口处理消息
void CDlgRemoteControl::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CMenu*  Menu = GetSystemMenu(FALSE);
	switch (nID)
	{
	case  IDM_REMOTE_CONTROL:
	{
		m_IsControl = !m_IsControl;
		//设置菜单样式
		Menu->CheckMenuItem(IDM_REMOTE_CONTROL, m_IsControl ? MF_CHECKED : MF_UNCHECKED);
		break;
	}
	case  IDM_REMOTE_CONTROL_TRACE_CURSOR:
	{
		m_IsTraceCursor = !m_IsTraceCursor;
		Menu->CheckMenuItem(IDM_REMOTE_CONTROL_TRACE_CURSOR, m_IsTraceCursor ? MF_CHECKED : MF_UNCHECKED);
		break;
	}
	case  IDM_REMOTE_CONTROL_BLOCK_INPUT:
	{
		BOOL  IsChecked = Menu->GetMenuState(IDM_REMOTE_CONTROL_BLOCK_INPUT, MF_BYCOMMAND)&MF_CHECKED;
		Menu->CheckMenuItem(IDM_REMOTE_CONTROL_BLOCK_INPUT, IsChecked ? MF_UNCHECKED : MF_CHECKED);
		BYTE  IsToken[2];
		IsToken[0] = CLIENT_REMOTE_CONTROL_BLOCK_INPUT;
		IsToken[1] = !IsChecked;
		m_IOCPServer->OnPreSending(m_ContextObject, IsToken, sizeof(IsToken));
		break;
	}
	case  IDM_REMOTE_CONTROL_SAVE_DIB:
	{
		SaveSnapshot();
		break;
	}
	//获取客户端剪贴板内容
	case  IDM_REMOTE_CONTROL_GET_CLIPBOARD:
	{
		BYTE  IsToken = CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REQUIRE;
		m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, sizeof(IsToken));
		break;
	}
	//向客户端剪切板中设置当前计算机剪切板的内容
	case  IDM_REMOTE_CONTROL_SET_CLIPBOARD:
	{
		SendClipboardData();
		break;
	}
	}
	CDialog::OnSysCommand(nID, lParam);
}


BOOL CDlgRemoteControl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	//过滤键盘鼠标消息
	if (m_hAccel_TraceCursor)
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccel_TraceCursor, pMsg))
		{
			return(TRUE);
		}
	}
	if (m_hAccel_ControlScreen)
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccel_ControlScreen, pMsg))
		{
			return(TRUE);
		}
	}
	if (m_hAccel_SaveSnapshot)
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccel_SaveSnapshot, pMsg))
		{
			return(TRUE);
		}
	}
	if (m_hAccel_BlockInput)
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccel_BlockInput, pMsg))
		{
			return(TRUE);
		}
	}
	if (m_hAccel_GetClipBoard)
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccel_GetClipBoard, pMsg))
		{
			return(TRUE);
		}
	}
	if (m_hAccel_SetClipBoard)
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccel_SetClipBoard, pMsg))
		{
			return(TRUE);
		}
	}

	
#define  MAKEDWORD(h,l)   (((unsigned  long)h<<16) | l )
		switch (pMsg->message)
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
		case  WM_MOUSEWHEEL:
		{
			MSG  Msg;
			memcpy(&Msg, pMsg, sizeof(MSG));
			Msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_VScrollPosition, LOWORD(pMsg->lParam) + m_HScrollPosition);
			Msg.pt.x += m_HScrollPosition;
			Msg.pt.y += m_VScrollPosition;
			SendCommand(&Msg);
			break;
		}
		case  WM_KEYDOWN:
		case  WM_KEYUP:
		case  WM_SYSKEYDOWN:
		case  WM_SYSKEYUP:
		{
			if (pMsg->wParam != VK_LWIN && pMsg->wParam != VK_RWIN)
			{
				MSG  Msg;
				memcpy(&Msg, pMsg, sizeof(MSG));
				Msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_VScrollPosition, LOWORD(pMsg->lParam) + m_HScrollPosition);
				Msg.pt.x += m_HScrollPosition;
				Msg.pt.y += m_VScrollPosition;
				SendCommand(&Msg);
			}
			if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			{
				return TRUE;
			}
			break;
		}
		}

	return CDialog::PreTranslateMessage(pMsg);
}
VOID CDlgRemoteControl::SendCommand(MSG*  Msg)
{
	if (!m_IsControl)
	{
		return;
	}
	LPBYTE  BufferData = new  BYTE[sizeof(MSG) + 1];
	BufferData[0] = CLIENT_REMOTE_CONTROL_CONTROL_REQUIRE;
	memcpy(BufferData + 1, Msg, sizeof(MSG));
	m_IOCPServer->OnPreSending(m_ContextObject, BufferData, sizeof(MSG) + 1);
	delete[] BufferData;
}
BOOL   CDlgRemoteControl::SaveSnapshot()
{
	CString  strFileName = CTime::GetCurrentTime().Format("%Y-%m-%d_%H-%M-%S.bmp");
	CFileDialog   Dialog(FALSE, "bmp", strFileName, OFN_OVERWRITEPROMPT, "位图文件(*.bmp)|*.bmp", this);
	if (Dialog.DoModal() != IDOK)
	{
		return  FALSE;
	}
	BITMAPFILEHEADER  BitMapFileHeader;
	LPBITMAPINFO    BitMapInfo = m_BitmapInfo;
	CFile  File;
	if (!File.Open(Dialog.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		return  FALSE;
	}
	int  nbmiSize = sizeof(BITMAPINFO);
	BitMapFileHeader.bfType = ((WORD)('M' << 8) | 'B');
	BitMapFileHeader.bfSize = BitMapInfo->bmiHeader.biSizeImage + sizeof(BitMapFileHeader);
	BitMapFileHeader.bfReserved1 = 0;
	BitMapFileHeader.bfReserved2 = 0;
	BitMapFileHeader.bfOffBits = sizeof(BitMapFileHeader) + nbmiSize;
	File.Write(&BitMapFileHeader, sizeof(BitMapFileHeader));
	File.Write(BitMapInfo, nbmiSize);
	File.Write(m_BitmapData, BitMapInfo->bmiHeader.biSizeImage);
	File.Close();
	return  TRUE;
}

VOID   CDlgRemoteControl::UpdateClipboard(char* BufferData, ULONG  BufferLength)
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
VOID  CDlgRemoteControl::SendClipboardData()
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

	BufferData[0] = CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REPLY;
	memcpy(BufferData + 1, v5, BufferLength - 1);
	::GlobalUnlock(GlobalHandle);
	::CloseClipboard();
	m_IOCPServer->OnPreSending(m_ContextObject,(PBYTE)BufferData, BufferLength);
	delete[]  BufferData;
}

void CDlgRemoteControl::OnAcceleratorTraceCursor()
{
	// TODO: 在此添加命令处理程序代码
	CMenu*  Menu = GetSystemMenu(FALSE);
	m_IsTraceCursor = !m_IsTraceCursor;
	Menu->CheckMenuItem(IDM_REMOTE_CONTROL_TRACE_CURSOR, m_IsTraceCursor ? MF_CHECKED : MF_UNCHECKED);
	return;
}


void CDlgRemoteControl::OnAcceleratorControlScreen()
{
	// TODO: 在此添加命令处理程序代码
	CMenu*  Menu = GetSystemMenu(FALSE);
	m_IsControl = !m_IsControl;
	//设置菜单样式
	Menu->CheckMenuItem(IDM_REMOTE_CONTROL, m_IsControl ? MF_CHECKED : MF_UNCHECKED);
}


void CDlgRemoteControl::OnAcceleratorSaveDib()
{
	// TODO: 在此添加命令处理程序代码
	SaveSnapshot();
}


void CDlgRemoteControl::OnAcceleratorBlockInput()
{
	// TODO: 在此添加命令处理程序代码
	CMenu*  Menu = GetSystemMenu(FALSE);
	BOOL  IsChecked = Menu->GetMenuState(IDM_REMOTE_CONTROL_BLOCK_INPUT, MF_BYCOMMAND)&MF_CHECKED;
	Menu->CheckMenuItem(IDM_REMOTE_CONTROL_BLOCK_INPUT, IsChecked ? MF_UNCHECKED : MF_CHECKED);
	BYTE  IsToken[2];
	IsToken[0] = CLIENT_REMOTE_CONTROL_BLOCK_INPUT;
	IsToken[1] = !IsChecked;
	m_IOCPServer->OnPreSending(m_ContextObject, IsToken, sizeof(IsToken));
}


void CDlgRemoteControl::OnAcceleratorGetClipboard()
{
	// TODO: 在此添加命令处理程序代码
	BYTE  IsToken = CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REQUIRE;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, sizeof(IsToken));
}


void CDlgRemoteControl::OnAcceleratorSetClipboard()
{
	// TODO: 在此添加命令处理程序代码
	SendClipboardData();
}
