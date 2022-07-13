// DlgProcessShowThreadInfo.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgProcessShowThreadInfo.h"
#include "afxdialogex.h"
#include "Resource.h"

// CDlgProcessShowThreadInfo �Ի���

IMPLEMENT_DYNAMIC(CDlgProcessShowThreadInfo, CDialogEx)

CDlgProcessShowThreadInfo::CDlgProcessShowThreadInfo(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialogEx(IDD_DIALOG_PROCESS_SHOW_THREAD_INFO, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgProcessShowThreadInfo::~CDlgProcessShowThreadInfo()
{
}

void CDlgProcessShowThreadInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DIALOG_PROCESS_THREAD_INFO_SHOW, m_CListCtrl_Dialog_Process_Thread_Info_Show);
}


BEGIN_MESSAGE_MAP(CDlgProcessShowThreadInfo, CDialogEx)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgProcessShowThreadInfo ��Ϣ�������


BOOL CDlgProcessShowThreadInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	SetIcon(m_hIcon, FALSE);

	//���ͻ���IP��ʾ�ڱ�����
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int  ClientAddressLength = sizeof(ClientAddress);
	//�õ����ӵ�IP��ַ
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);
	v1.Format("\\\\%s - �����̹߳���", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);

	//���ض˴��ص�����
	char*  BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0));
	m_CListCtrl_Dialog_Process_Thread_Info_Show.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	switch (BufferData[0])
	{
	case  CLIENT_PROCESS_THREAD_INFO_SHOW_REPLY:
	{
		m_CListCtrl_Dialog_Process_Thread_Info_Show.InsertColumn(0, "�߳�ID", LVCFMT_LEFT, 70);
		m_CListCtrl_Dialog_Process_Thread_Info_Show.InsertColumn(1, "�̻߳ص���ַ", LVCFMT_LEFT, 120);
		m_CListCtrl_Dialog_Process_Thread_Info_Show.InsertColumn(2, "�߳�Teb", LVCFMT_LEFT, 120);
		m_CListCtrl_Dialog_Process_Thread_Info_Show.InsertColumn(3, "�߳����ȼ�", LVCFMT_LEFT, 120);
		ShowProcessThreadList();
		break;
	}
	default:
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}
void  CDlgProcessShowThreadInfo::ShowProcessThreadList(void)
{
	char*  BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));
	PVOID  ThreadStartAddress = NULL;
	PVOID   ThreadTeb = NULL;
	KPRIORITY  ThreadPriority = 0;
	DWORD  Offset = 0;
	CString v1;
	m_CListCtrl_Dialog_Process_Thread_Info_Show.DeleteAllItems();
	int i;
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength(); i++)
	{
		HANDLE* ThreadID = (HANDLE*)(BufferData + Offset);                                   //����ID
	//	ThreadStartAddress = (PVOID*)(BufferData + Offset + sizeof(HANDLE));                 //�̻߳ص���ַ
		memcpy(&ThreadStartAddress, BufferData + Offset + sizeof(HANDLE), sizeof(PVOID));
		ThreadTeb = (PVOID)(BufferData + Offset + sizeof(HANDLE) + sizeof(PVOID));                             //�߳�Teb
	//	memcpy(&ThreadTeb, BufferData + Offset + sizeof(HANDLE) + sizeof(PVOID), sizeof(PVOID));
		memcpy(&ThreadPriority, BufferData + Offset + sizeof(HANDLE) + sizeof(PVOID) + sizeof(PVOID), sizeof(KPRIORITY));
	//	ThreadPriority = (KPRIORITY*)(ThreadTeb + sizeof(PVOID));                              //�߳����ȼ�

		v1.Format("%5u", *ThreadID);
		m_CListCtrl_Dialog_Process_Thread_Info_Show.InsertItem(i, v1);

		v1.Format("0x%016llx", ThreadStartAddress);
		m_CListCtrl_Dialog_Process_Thread_Info_Show.SetItemText(i, 1, v1);

		v1.Format("0x%016llx", ThreadTeb);
		m_CListCtrl_Dialog_Process_Thread_Info_Show.SetItemText(i, 2, v1);

		v1.Format("%d", ThreadPriority);
		m_CListCtrl_Dialog_Process_Thread_Info_Show.SetItemText(i, 3, v1);

	
		//����ID��Ϊ����
		m_CListCtrl_Dialog_Process_Thread_Info_Show.SetItemData(i, (DWORD_PTR)*ThreadID);
		//������һ��ѭ��
		Offset += sizeof(HANDLE) + sizeof(PVOID) + sizeof(PVOID) + sizeof(KPRIORITY);
	}

}


void CDlgProcessShowThreadInfo::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	/*CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);*/
	CDialogEx::OnClose();
}
