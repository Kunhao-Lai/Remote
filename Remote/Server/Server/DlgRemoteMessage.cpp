// DlgRemoteMessage.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRemoteMessage.h"
#include "afxdialogex.h"


// CDlgRemoteMessage �Ի���

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


// CDlgRemoteMessage ��Ϣ�������


BOOL CDlgRemoteMessage::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	SetIcon(m_hIcon, FALSE);
	BYTE  IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CDlgRemoteMessage::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}


BOOL CDlgRemoteMessage::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
    {
		//���ESC���˳�����
		if (pMsg->wParam == VK_ESCAPE)
		{
			PostMessage(WM_CLOSE);
			return TRUE;
		}
		//����Ƿ��Ǳ༭��Ļس���
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
	//���m_CEdit_Dialog_Remote_Message�ؼ��ϵ����ݳ���
	int  BufferLength = m_CEdit_Dialog_Remote_Message.GetWindowTextLength();
	if (!BufferLength)
	{
		return;
	}
	//��ÿؼ��ϵ�����
	CString  v1;
	m_CEdit_Dialog_Remote_Message.GetWindowText(v1);
	//IOͨѶ�׽���ֻ֧��char�����ݣ����Խ�v1������ݿ���BufferData��
	char* BufferData = (char*)VirtualAlloc(NULL, BufferLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	memset(BufferData, 0, sizeof(char)*BufferLength);
	sprintf(BufferData, "%s", v1.GetBuffer(0));

	//�ؼ�����������
	m_CEdit_Dialog_Remote_Message.SetWindowText(NULL);

	//����ͨѶ�������еķ������ݺ���
	m_IOCPServer->OnPreSending(m_ContextObject,
		(LPBYTE)BufferData, strlen(BufferData));
	if (BufferData != NULL)
	{
		VirtualFree(BufferData, BufferLength, MEM_RELEASE);
		BufferData = NULL;
	}
}
