// DlgCreateClient.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgCreateClient.h"
#include "afxdialogex.h"


struct  _SERVER_CONNECT_INFORMATION_
{
	DWORD   CheckFlag;
	char    ServerIP[20];
	USHORT  ServerConnectPort;
}__ServerConnectInfo = {0x87654321,"",0 };


// CDlgCreateClient �Ի���

IMPLEMENT_DYNAMIC(CDlgCreateClient, CDialog)

CDlgCreateClient::CDlgCreateClient(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_CREATE_CLIENT, pParent)
	, m_CEdit_Dialog_Create_Client_IP(_T("����������IP"))
	, m_CEdit_Dialog_Create_Client_Port(_T("������˿ں�"))
{

}

CDlgCreateClient::~CDlgCreateClient()
{
}

void CDlgCreateClient::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DIALOG_CREATE_CLIENT_IP, m_CEdit_Dialog_Create_Client_IP);
	DDX_Text(pDX, IDC_EDIT_DIALOG_CREATE_CLIENT_PORT, m_CEdit_Dialog_Create_Client_Port);
}


BEGIN_MESSAGE_MAP(CDlgCreateClient, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgCreateClient::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgCreateClient ��Ϣ�������


void CDlgCreateClient::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CFile  FileObject;
	//�����ڿؼ��ϵ����ݸ��µ��ؼ�������
	UpdateData(TRUE);
	USHORT  SeverPort = atoi(m_CEdit_Dialog_Create_Client_Port);
	strcpy(__ServerConnectInfo.ServerIP, m_CEdit_Dialog_Create_Client_IP);
	if (SeverPort < 0 || SeverPort>65536)
	{
		__ServerConnectInfo.ServerConnectPort = 8888;
	}
	else
	{
		__ServerConnectInfo.ServerConnectPort = SeverPort;
	}
	char  v2[MAX_PATH];
	ZeroMemory(v2, MAX_PATH);
	LONGLONG  BufferLength = 0;
	BYTE*     BufferData = NULL;
	CString   v3;
	CString   FileFullPath;
	try
	{
		//�˴��õ�δ������ļ���
		GetModuleFileName(NULL, v2, MAX_PATH);
		v3 = v2;
		//���������ѯ��һ�γ��ֵ�'\\'

		int  Position = v3.ReverseFind('\\');
		v3 = v3.Left(Position);
		//�õ���ǰδ�����ļ���
		FileFullPath = v3 + "\\Client.exe";
		//���ļ�
		FileObject.Open(FileFullPath, CFile::modeRead | CFile::typeBinary);
		BufferLength = FileObject.GetLength();
		BufferData = new BYTE[BufferLength];
		ZeroMemory(BufferData, BufferLength);
		//��ȡ�ļ�����
		FileObject.Read(BufferData, BufferLength);
		//�ر��ļ�
		FileObject.Close();

		//���ڴ�������0x87654321
		int  Offset = SeMemoryFind((char*)BufferData, (char*)&__ServerConnectInfo.CheckFlag,
			BufferLength, sizeof(DWORD));
		//���Լ����õ���Ϣ������exe���ڴ���
		memcpy(BufferData + Offset, &__ServerConnectInfo, sizeof(__ServerConnectInfo));
		//�������ļ�
		FileObject.Open(FileFullPath, CFile::modeRead | CFile::typeBinary | CFile::modeWrite);
		FileObject.Write(BufferData, BufferLength);
		//�ر��ļ�
		FileObject.Close();
		delete[]  BufferData;
		MessageBox("���ɳɹ�");

	}
	catch (CMemoryException* e)
	{
		MessageBox("�ڴ治��");
	}
	catch (CFileException* e)
	{
		MessageBox("�ļ���������");
	}
	catch (CException* e)
	{
		MessageBox("δ֪����");
	}
	CDialog::OnOK();
}
int  CDlgCreateClient::SeMemoryFind(const char* BufferData, const char*  KeyValue, int BufferLength, int KeyLength)
{
	int i = 0; 
	int j = 0;
	if (KeyLength == 0 || BufferLength == 0)
	{
		return -1;
	}
	for (i = 0; i < BufferLength; i++)
	{
		for (j = 0; j < KeyLength; j++)
		{
			if (BufferData[i + j] != KeyValue[j])
			{
				break;
			}
		}
		if (j == KeyLength)
		{
			return i;
		}
	}
	return -1;
}
