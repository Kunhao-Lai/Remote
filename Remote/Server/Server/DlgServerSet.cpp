// DlgServerSet.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgServerSet.h"
#include "afxdialogex.h"
#include "ConfigFile.h"

// CDlgServerSet 对话框
extern   CConfigFile   __ConfigFile;

IMPLEMENT_DYNAMIC(CDlgServerSet, CDialog)

CDlgServerSet::CDlgServerSet(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_SERVER_SET, pParent)
	, m_CEdit_Server_Set_MaxConnection(0)
	, m_CEdit_Server_Set_Port(0)
{

}

CDlgServerSet::~CDlgServerSet()
{
}

void CDlgServerSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_CEdit_Server_Set_MaxConnection);
	DDX_Text(pDX, IDC_EDIT2, m_CEdit_Server_Set_Port);
	DDV_MinMaxLong(pDX, m_CEdit_Server_Set_MaxConnection, 1, 20);
	DDV_MinMaxLong(pDX, m_CEdit_Server_Set_Port, 2048, 65536);
	DDX_Control(pDX, IDC_BUTTON_APPLY, m_CButton_Server_Set_Apply);
}


BEGIN_MESSAGE_MAP(CDlgServerSet, CDialog)
	ON_EN_CHANGE(IDC_EDIT1, &CDlgServerSet::OnEnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, &CDlgServerSet::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CDlgServerSet::OnBnClickedButtonApply)
END_MESSAGE_MAP()


// CDlgServerSet 消息处理程序


void CDlgServerSet::OnEnChangeEdit1()
{
	m_CButton_Server_Set_Apply.ShowWindow(SW_NORMAL);
	m_CButton_Server_Set_Apply.EnableWindow(TRUE);
}


void CDlgServerSet::OnEnChangeEdit2()
{
	m_CButton_Server_Set_Apply.ShowWindow(SW_NORMAL);
	m_CButton_Server_Set_Apply.EnableWindow(TRUE);
}


BOOL CDlgServerSet::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	int ListenPort = 0;
	int MaxConnection = 0;
	//将数据设置到窗口
	ListenPort = __ConfigFile.GetInt("Settings", "ListenPort");  //读取ini文件中的监听端口
	MaxConnection = __ConfigFile.GetInt("Settings", "MaxConnection");   //读取ini文件中的最大连接数
	m_CEdit_Server_Set_Port = ListenPort;
	m_CEdit_Server_Set_MaxConnection = MaxConnection;
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgServerSet::OnBnClickedButtonApply()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	__ConfigFile.SetInt("Settings", "ListenPort", m_CEdit_Server_Set_Port);
	__ConfigFile.SetInt("Settings", "MaxConnection", m_CEdit_Server_Set_MaxConnection);
	SendMessage(WM_CLOSE);
}
