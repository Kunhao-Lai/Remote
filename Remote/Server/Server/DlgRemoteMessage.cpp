// DlgRemoteMessage.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRemoteMessage.h"
#include "afxdialogex.h"


// CDlgRemoteMessage 对话框

IMPLEMENT_DYNAMIC(CDlgRemoteMessage, CDialog)

CDlgRemoteMessage::CDlgRemoteMessage(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialog(IDD_DIALIG_REMOTE_MESSAGE, pParent)
{
	
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgRemoteMessage::~CDlgRemoteMessage()
{
}

void CDlgRemoteMessage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DIALOG_REMOTE_MESSAGE, m_CEdit_Dialog_Remote_Message);
}


BEGIN_MESSAGE_MAP(CDlgRemoteMessage, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgRemoteMessage 消息处理程序


BOOL CDlgRemoteMessage::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	SetIcon(m_hIcon, FALSE);
	BYTE  IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgRemoteMessage::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}


BOOL CDlgRemoteMessage::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
    {
		//点击ESC键退出窗口
		if (pMsg->wParam == VK_ESCAPE)
		{
			PostMessage(WM_CLOSE);
			return TRUE;
		}
		//如果是否是编辑框的回车键
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_CEdit_Dialog_Remote_Message.m_hWnd)
		{
			OnSending();
			return  TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
void   CDlgRemoteMessage::OnSending()
{
	//获得m_CEdit_Dialog_Remote_Message控件上的数据长度
	int  BufferLength = m_CEdit_Dialog_Remote_Message.GetWindowTextLength();
	if (!BufferLength)
	{
		return;
	}
	//获得控件上的数据
	CString  v1;
	m_CEdit_Dialog_Remote_Message.GetWindowText(v1);
	//IO通讯套接字只支持char型数据，所以将v1里的数据拷到BufferData里
	char* BufferData = (char*)VirtualAlloc(NULL, BufferLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	memset(BufferData, 0, sizeof(char)*BufferLength);
	sprintf(BufferData, "%s", v1.GetBuffer(0));

	//控件里的数据清空
	m_CEdit_Dialog_Remote_Message.SetWindowText(NULL);

	//调用通讯引擎类中的发送数据函数
	m_IOCPServer->OnPreSending(m_ContextObject,
		(LPBYTE)BufferData, strlen(BufferData));
	if (BufferData != NULL)
	{
		VirtualFree(BufferData, BufferLength, MEM_RELEASE);
		BufferData = NULL;
	}
}
