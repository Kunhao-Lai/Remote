// DlgCmdManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgCmdManager.h"
#include "afxdialogex.h"
#include"Common.h"

// CDlgCmdManager 对话框

IMPLEMENT_DYNAMIC(CDlgCmdManager, CDialog)

CDlgCmdManager::CDlgCmdManager(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialog(IDD_DIALOG_CMD_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgCmdManager::~CDlgCmdManager()
{
}

void CDlgCmdManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DIALOG_CMD_MANAGER_SHOW, m_CEdit_Dialog_Cmd_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgCmdManager, CDialog)
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CDlgCmdManager 消息处理程序


BOOL CDlgCmdManager::OnInitDialog()
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
	v1.Format("\\\\%s - 远程终端管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	CFont* m_Font = new CFont;
	m_Font->CreateFont(-11, 0, 0, 0, 100, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "楷体");
	CEdit *m_Edit = (CEdit*)GetDlgItem(IDC_EDIT_DIALOG_CMD_MANAGER_SHOW);
	m_Edit->SetFont(m_Font, FALSE);

	//设置对话框上的数据
	LOGFONT  Logfont;
	CFont*  v2 = m_CEdit_Dialog_Cmd_Manager_Show.GetFont();
	v2->GetLogFont(&Logfont);
	Logfont.lfHeight = Logfont.lfHeight*1.3;      //此处可以修改字体的高比例
	Logfont.lfWidth = Logfont.lfWidth*1.3;        //此处可以修改字体的宽比例
	CFont  v4;
	v4.CreateFontIndirect(&Logfont);
	m_CEdit_Dialog_Cmd_Manager_Show.SetFont(&v4);
	v4.Detach();

	BYTE  IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgCmdManager::OnClose()
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
void  CDlgCmdManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	ShowCmdData();
	m_ShowDataLength = m_CEdit_Dialog_Cmd_Manager_Show.GetWindowTextLength();
	
}
void   CDlgCmdManager::ShowCmdData()
{
	//从客户端来的数据要加上一个\0
	m_ContextObject->m_ReceivedDecompressedBufferData.WriteArray((LPBYTE)"", 1);
	//获得所有数据，包括 \0
	CString  v1 = (char*)m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0);
	//替换掉原来的换行符，可能cmd的换行同Win32编辑框不一样
	v1.Replace("\n", "\r\n");
	//得到当前窗口的字符个数
	int BufferLength = m_CEdit_Dialog_Cmd_Manager_Show.GetWindowTextLength();
	//将光标定位到该位置并选中指定个数的字符 ，也就是末尾，因为从被控端来的数据要显示在我们先前内容的前面
	m_CEdit_Dialog_Cmd_Manager_Show.SetSel(BufferLength, BufferLength);

	//显示数据
	m_CEdit_Dialog_Cmd_Manager_Show.ReplaceSel(v1);
	//重新得到字符的大小
	m_911 = m_CEdit_Dialog_Cmd_Manager_Show.GetWindowTextLength();
}

BOOL CDlgCmdManager::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		//点击ESC键退出窗口
		if (pMsg->wParam == VK_ESCAPE|| pMsg->wParam == VK_DELETE)
		{
			return TRUE;
		}
		//如果是否是编辑框的回车键
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_CEdit_Dialog_Cmd_Manager_Show.m_hWnd)
		{
			//得到窗口的数据大小
			int  BufferLength = m_CEdit_Dialog_Cmd_Manager_Show.GetWindowTextLength();
			CString  BufferData;
			//得到窗口的字符数据
			m_CEdit_Dialog_Cmd_Manager_Show.GetWindowText(BufferData);
			BufferData += "\r\n";
			m_IOCPServer->OnPreSending(m_ContextObject, (LPBYTE)BufferData.GetBuffer(0) + m_911,
				BufferData.GetLength() - m_911);
			m_911 = m_CEdit_Dialog_Cmd_Manager_Show.GetWindowTextLength();
		}
		if (pMsg->wParam == VK_BACK && pMsg->hwnd == m_CEdit_Dialog_Cmd_Manager_Show.m_hWnd)
		{
			if (m_CEdit_Dialog_Cmd_Manager_Show.GetWindowTextLength() <= m_ShowDataLength)
			{
				return TRUE;
			}
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CDlgCmdManager::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if ((pWnd->GetDlgCtrlID() == IDC_EDIT_DIALOG_CMD_MANAGER_SHOW) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF  ColorReference = RGB(255, 255, 255);
		pDC->SetTextColor(ColorReference);          //设置白色的文本
		ColorReference = RGB(0, 0, 0);
		pDC->SetBkColor(ColorReference);           //设置黑色的背景
		return  CreateSolidBrush(ColorReference);             //作为约定，返回背景色对应的刷子句柄
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
