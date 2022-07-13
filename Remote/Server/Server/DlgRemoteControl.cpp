// DlgRemoteControl.cpp : ʵ���ļ�
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
	IDM_REMOTE_CONTROL_TRACE_CURSOR,                //������ʾԶ�����
	IDM_REMOTE_CONTROL_BLOCK_INPUT,                 //����Զ�̼��������
	IDM_REMOTE_CONTROL_SAVE_DIB,                    //����ͼƬ
	IDM_REMOTE_CONTROL_GET_CLIPBOARD,               //��ȡ������
	IDM_REMOTE_CONTROL_SET_CLIPBOARD,               //���ü�����
};
// CDlgRemoteControl �Ի���

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
	//����λͼͷ��Ϣ
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
	//���չ�����
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


// CDlgRemoteControl ��Ϣ�������


BOOL CDlgRemoteControl::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	SetIcon(m_hIcon, FALSE);

	//���ͻ���IP��ʾ�ڱ�����
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int  ClientAddressLength = sizeof(ClientAddress);
	//�õ����ӵ�IP��ַ
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);
	v1.Format("\\\\%s - Զ�̿���", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);

	m_DesktopDCHandle = ::GetDC(m_hWnd);
	m_DesktopMemoryDCHandle = CreateCompatibleDC(m_DesktopDCHandle);
	m_BitmapHandle = CreateDIBSection(m_DesktopDCHandle, m_BitmapInfo,
		DIB_RGB_COLORS, &m_BitmapData, NULL, NULL);
	//��ʼ��������
	SelectObject(m_DesktopMemoryDCHandle, m_BitmapHandle);

	SetWindowPos(NULL, 0, 0, m_BitmapInfo->bmiHeader.biWidth / 2, m_BitmapInfo->bmiHeader.biHeight / 2, 0);

	SetScrollRange(SB_HORZ, 0, m_BitmapInfo->bmiHeader.biWidth / 1.8);
	SetScrollRange(SB_VERT, 0, m_BitmapInfo->bmiHeader.biHeight / 1.8);

	//���ϵͳ�˵�
	CMenu*  SysMenu = GetSystemMenu(FALSE);
	if (SysMenu != NULL)
	{
		SysMenu->AppendMenu(MF_SEPARATOR);
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL,"������Ļ(&Y)");               //��ݼ�
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL_TRACE_CURSOR, "���ٱ��ض����(&T)");
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL_BLOCK_INPUT, "�������ض����ͼ���(&L)");
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL_SAVE_DIB, "�������(&S)");
		SysMenu->AppendMenu(MF_SEPARATOR);
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL_GET_CLIPBOARD, "��ȡ������(&R)");
		SysMenu->AppendMenu(MF_STRING, IDM_REMOTE_CONTROL_SET_CLIPBOARD, "���ü�����(&F)");
		SysMenu->AppendMenu(MF_SEPARATOR);
	}
	//�ͻ�����Ļ�е����λ��
	m_ClientCursorPosition.x = 0;
	m_ClientCursorPosition.y = 0;
	//�ش�����
	BYTE  IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CDlgRemoteControl::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
		//�����ܵ��Ŀͻ������ݷ��뵽���ض˵ļ��а�
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
	//��ʶ+�㷨+���λ��+�������
	ULONG  v1 = 1 + 1 + sizeof(POINT) + sizeof(BYTE);
	LPVOID  PreScreenData = m_BitmapData;
	LPVOID  CurrentBufferData = m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(v1);
	ULONG   CurrentBufferLength = m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - v1;
	POINT   PreClientCursorPos;
	memcpy(&PreClientCursorPos, &m_ClientCursorPosition, sizeof(POINT));
	memcpy(&m_ClientCursorPosition, m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(2), sizeof(POINT));
	//����ƶ���
	if (memcmp(&PreClientCursorPos, &m_ClientCursorPosition, sizeof(POINT)) != 0)
	{
		IsChange = TRUE;
	}
	//��Ļ�Ƿ��б仯
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
					   // TODO: �ڴ˴������Ϣ����������
					   // ��Ϊ��ͼ��Ϣ���� CDialog::OnPaint()
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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

//����Ϣ���հ���ϵͳ�˵��ȴ����ڴ�����Ϣ
void CDlgRemoteControl::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CMenu*  Menu = GetSystemMenu(FALSE);
	switch (nID)
	{
	case  IDM_REMOTE_CONTROL:
	{
		m_IsControl = !m_IsControl;
		//���ò˵���ʽ
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
	//��ȡ�ͻ��˼���������
	case  IDM_REMOTE_CONTROL_GET_CLIPBOARD:
	{
		BYTE  IsToken = CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REQUIRE;
		m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, sizeof(IsToken));
		break;
	}
	//��ͻ��˼��а������õ�ǰ��������а������
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
	// TODO: �ڴ����ר�ô����/����û���
	//���˼��������Ϣ
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
	CFileDialog   Dialog(FALSE, "bmp", strFileName, OFN_OVERWRITEPROMPT, "λͼ�ļ�(*.bmp)|*.bmp", this);
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
	//��ռ���������
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
	// TODO: �ڴ���������������
	CMenu*  Menu = GetSystemMenu(FALSE);
	m_IsTraceCursor = !m_IsTraceCursor;
	Menu->CheckMenuItem(IDM_REMOTE_CONTROL_TRACE_CURSOR, m_IsTraceCursor ? MF_CHECKED : MF_UNCHECKED);
	return;
}


void CDlgRemoteControl::OnAcceleratorControlScreen()
{
	// TODO: �ڴ���������������
	CMenu*  Menu = GetSystemMenu(FALSE);
	m_IsControl = !m_IsControl;
	//���ò˵���ʽ
	Menu->CheckMenuItem(IDM_REMOTE_CONTROL, m_IsControl ? MF_CHECKED : MF_UNCHECKED);
}


void CDlgRemoteControl::OnAcceleratorSaveDib()
{
	// TODO: �ڴ���������������
	SaveSnapshot();
}


void CDlgRemoteControl::OnAcceleratorBlockInput()
{
	// TODO: �ڴ���������������
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
	// TODO: �ڴ���������������
	BYTE  IsToken = CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REQUIRE;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, sizeof(IsToken));
}


void CDlgRemoteControl::OnAcceleratorSetClipboard()
{
	// TODO: �ڴ���������������
	SendClipboardData();
}
