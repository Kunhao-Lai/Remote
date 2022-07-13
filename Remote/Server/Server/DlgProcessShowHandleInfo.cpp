// DlgProcessShowHandleInfo.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgProcessShowHandleInfo.h"
#include "afxdialogex.h"
#include "Resource.h"

// CDlgProcessShowHandleInfo �Ի���

IMPLEMENT_DYNAMIC(CDlgProcessShowHandleInfo, CDialogEx)

CDlgProcessShowHandleInfo::CDlgProcessShowHandleInfo(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialogEx(IDD_DIALOG_PROCESS_SHOW_HANDLE_INFO, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgProcessShowHandleInfo::~CDlgProcessShowHandleInfo()
{
}

void CDlgProcessShowHandleInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DIALOG_PROCESS_HANDLE_INFO_SHOW, m_CListCtrl_Dialog_Process_Handle_Info_Show);
}


BEGIN_MESSAGE_MAP(CDlgProcessShowHandleInfo, CDialogEx)
END_MESSAGE_MAP()


// CDlgProcessShowHandleInfo ��Ϣ�������


BOOL CDlgProcessShowHandleInfo::OnInitDialog()
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
	v1.Format("\\\\%s - ���̾������", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);

	//���ض˴��ص�����
	char*  BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0));
	m_CListCtrl_Dialog_Process_Handle_Info_Show.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	switch (BufferData[0])
	{
	case  CLIENT_PROCESS_THREAD_INFO_SHOW_REPLY:
	{
		m_CListCtrl_Dialog_Process_Handle_Info_Show.InsertColumn(0, "�������", LVCFMT_LEFT, 70);
		m_CListCtrl_Dialog_Process_Handle_Info_Show.InsertColumn(1, "�������", LVCFMT_LEFT, 120);
		m_CListCtrl_Dialog_Process_Handle_Info_Show.InsertColumn(2, "���ֵ", LVCFMT_LEFT, 120);
		m_CListCtrl_Dialog_Process_Handle_Info_Show.InsertColumn(3, "�������", LVCFMT_LEFT, 120);
		ShowProcessHandleList();
		break;
	}
	default:
		break;
	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}
void  CDlgProcessShowHandleInfo::ShowProcessHandleList(void)
{
	char*  BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));
	USHORT HandleValue = 0;
	PVOID  Object = NULL;
	string HandleName = NULL;
	string HandleType = NULL;
	DWORD  Offset = 0;
	CString v1;
	m_CListCtrl_Dialog_Process_Handle_Info_Show.DeleteAllItems();
	int i;
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength(); i++)
	{
	                       	
		memcpy(&HandleName, BufferData + Offset , sizeof(string));
		memcpy(&HandleType, BufferData + Offset + sizeof(string) , sizeof(string));
		memcpy(&HandleValue, BufferData + Offset + sizeof(string) + sizeof(string), sizeof(USHORT));
		memcpy(&Object, BufferData + Offset + sizeof(string) + sizeof(string) + sizeof(USHORT), sizeof(PVOID));

		v1.Format("%s", HandleName);
		m_CListCtrl_Dialog_Process_Handle_Info_Show.InsertItem(i, v1);

		v1.Format("%s", HandleType);
		m_CListCtrl_Dialog_Process_Handle_Info_Show.SetItemText(i, 1, v1);

		v1.Format("%d", HandleValue);
		m_CListCtrl_Dialog_Process_Handle_Info_Show.SetItemText(i, 2, v1);

		v1.Format("0x%016llx", Object);
		m_CListCtrl_Dialog_Process_Handle_Info_Show.SetItemText(i, 3, v1);

		//������һ��ѭ��
		Offset += sizeof(string) + sizeof(string) + sizeof(PVOID) + sizeof(USHORT);
	}

}