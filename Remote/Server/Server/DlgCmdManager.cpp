// DlgCmdManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgCmdManager.h"
#include "afxdialogex.h"
#include"Common.h"

// CDlgCmdManager �Ի���

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


// CDlgCmdManager ��Ϣ�������


BOOL CDlgCmdManager::OnInitDialog()
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
	v1.Format("\\\\%s - Զ���ն˹���", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);

	CFont* m_Font = new CFont;
	m_Font->CreateFont(-11, 0, 0, 0, 100, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "����");
	CEdit *m_Edit = (CEdit*)GetDlgItem(IDC_EDIT_DIALOG_CMD_MANAGER_SHOW);
	m_Edit->SetFont(m_Font, FALSE);

	//���öԻ����ϵ�����
	LOGFONT  Logfont;
	CFont*  v2 = m_CEdit_Dialog_Cmd_Manager_Show.GetFont();
	v2->GetLogFont(&Logfont);
	Logfont.lfHeight = Logfont.lfHeight*1.3;      //�˴������޸�����ĸ߱���
	Logfont.lfWidth = Logfont.lfWidth*1.3;        //�˴������޸�����Ŀ����
	CFont  v4;
	v4.CreateFontIndirect(&Logfont);
	m_CEdit_Dialog_Cmd_Manager_Show.SetFont(&v4);
	v4.Detach();

	BYTE  IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CDlgCmdManager::OnClose()
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
	//�ӿͻ�����������Ҫ����һ��\0
	m_ContextObject->m_ReceivedDecompressedBufferData.WriteArray((LPBYTE)"", 1);
	//����������ݣ����� \0
	CString  v1 = (char*)m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0);
	//�滻��ԭ���Ļ��з�������cmd�Ļ���ͬWin32�༭��һ��
	v1.Replace("\n", "\r\n");
	//�õ���ǰ���ڵ��ַ�����
	int BufferLength = m_CEdit_Dialog_Cmd_Manager_Show.GetWindowTextLength();
	//����궨λ����λ�ò�ѡ��ָ���������ַ� ��Ҳ����ĩβ����Ϊ�ӱ��ض���������Ҫ��ʾ��������ǰ���ݵ�ǰ��
	m_CEdit_Dialog_Cmd_Manager_Show.SetSel(BufferLength, BufferLength);

	//��ʾ����
	m_CEdit_Dialog_Cmd_Manager_Show.ReplaceSel(v1);
	//���µõ��ַ��Ĵ�С
	m_911 = m_CEdit_Dialog_Cmd_Manager_Show.GetWindowTextLength();
}

BOOL CDlgCmdManager::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		//���ESC���˳�����
		if (pMsg->wParam == VK_ESCAPE|| pMsg->wParam == VK_DELETE)
		{
			return TRUE;
		}
		//����Ƿ��Ǳ༭��Ļس���
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_CEdit_Dialog_Cmd_Manager_Show.m_hWnd)
		{
			//�õ����ڵ����ݴ�С
			int  BufferLength = m_CEdit_Dialog_Cmd_Manager_Show.GetWindowTextLength();
			CString  BufferData;
			//�õ����ڵ��ַ�����
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

	// TODO:  �ڴ˸��� DC ���κ�����
	if ((pWnd->GetDlgCtrlID() == IDC_EDIT_DIALOG_CMD_MANAGER_SHOW) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF  ColorReference = RGB(255, 255, 255);
		pDC->SetTextColor(ColorReference);          //���ð�ɫ���ı�
		ColorReference = RGB(0, 0, 0);
		pDC->SetBkColor(ColorReference);           //���ú�ɫ�ı���
		return  CreateSolidBrush(ColorReference);             //��ΪԼ�������ر���ɫ��Ӧ��ˢ�Ӿ��
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}
