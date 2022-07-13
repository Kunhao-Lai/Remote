// DlgServiceManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgServiceManager.h"
#include "afxdialogex.h"


// CDlgServiceManager 对话框

IMPLEMENT_DYNAMIC(CDlgServiceManager, CDialog)

CDlgServiceManager::CDlgServiceManager(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialog(IDD_DIALOG_SERVICE_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgServiceManager::~CDlgServiceManager()
{
}

void CDlgServiceManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DIALOG_SERVICE_MANAGER_SHOW, m_CListCtrl_Dialog_Service_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgServiceManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_DIALOG_SERVICE_MANAGER_SHOW, &CDlgServiceManager::OnNMRClickListDialogServiceManagerShow)
	ON_COMMAND(ID_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_REFRESH, &CDlgServiceManager::OnMenuDialogServiceManagerListShowRefresh)
	ON_COMMAND(ID_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_START, &CDlgServiceManager::OnMenuDialogServiceManagerListShowStart)
	ON_COMMAND(ID_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_STOP, &CDlgServiceManager::OnMenuDialogServiceManagerListShowStop)
	ON_COMMAND(ID_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_START_AUTO, &CDlgServiceManager::OnMenuDialogServiceManagerListShowStartAuto)
	ON_COMMAND(ID_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW_START_MANUAL, &CDlgServiceManager::OnMenuDialogServiceManagerListShowStartManual)
END_MESSAGE_MAP()


// CDlgServiceManager 消息处理程序


void CDlgServiceManager::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_ContextObject->DialogHandle = NULL;
	m_ContextObject->DialogID = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}


BOOL CDlgServiceManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// TODO:  在此添加额外的初始化
	SetIcon(m_hIcon, FALSE);

	//将客户端IP显示在标题栏
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int  ClientAddressLength = sizeof(ClientAddress);
	//得到连接的IP地址
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);
	v1.Format("\\\\%s - 远程服务管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	m_CListCtrl_Dialog_Service_Manager_Show.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(0, "真实名称", LVCFMT_LEFT, 120);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(1, "显示名称", LVCFMT_LEFT, 120);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(2, "启动类型", LVCFMT_LEFT, 40);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(3, "运行状态", LVCFMT_LEFT, 40);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(4, "可执行文件路径", LVCFMT_LEFT, 120);

	ShowClientServiceList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
int  CDlgServiceManager::ShowClientServiceList(void)
{
	char*  BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));
	char*  DisplayName;
	char*  ServiceName;
	char*  RunWay;
	char*  AutoRun;
	char*  FileFullPath;
	DWORD  Offset = 0;
	m_CListCtrl_Dialog_Service_Manager_Show.DeleteAllItems();
	int i = 0;
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1; i++)
	{
		DisplayName = BufferData + Offset;
		ServiceName = DisplayName + lstrlen(DisplayName) + 1;
		FileFullPath = ServiceName + lstrlen(ServiceName) + 1;
		RunWay = FileFullPath + lstrlen(FileFullPath) + 1;
		AutoRun = RunWay + lstrlen(RunWay) + 1;
		m_CListCtrl_Dialog_Service_Manager_Show.InsertItem(i, ServiceName);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 1, DisplayName);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 2, AutoRun);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 3, RunWay);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 4, FileFullPath);

		Offset += lstrlen(DisplayName) + lstrlen(ServiceName) + lstrlen(FileFullPath) + lstrlen(RunWay) + lstrlen(AutoRun) + 5;
	}
	CString  v1;
	v1.Format("服务个数:%d", i);
	LVCOLUMN  v3;
	v3.mask = LVCF_TEXT;
	v3.pszText = v1.GetBuffer(0);
	v3.cchTextMax = v1.GetLength();
	m_CListCtrl_Dialog_Service_Manager_Show.SetColumn(4, &v3);
	return 0;
}

void CDlgServiceManager::OnNMRClickListDialogServiceManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu  Menu;
	Menu.LoadMenu(IDR_MENU_DIALOG_SERVICE_MANAGER_LIST_SHOW);
	CMenu*  SubMenu = Menu.GetSubMenu(0);
	CPoint  Point;
	GetCursorPos(&Point);
	SubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, Point.x, Point.y, this);
	*pResult = 0;
}


void CDlgServiceManager::OnMenuDialogServiceManagerListShowRefresh()
{
	// TODO: 在此添加命令处理程序代码
	BYTE  IsToken = CLIENT_SERVICE_MANAGER_REQUIRE;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, 1);
}
void  CDlgServiceManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case  CLIENT_SERVICE_MANAGER_REPLY:
	{
		ShowClientServiceList();
		break;
	}
	default:
		break;
	}
}


void CDlgServiceManager::OnMenuDialogServiceManagerListShowStart()
{
	// TODO: 在此添加命令处理程序代码
	ConfigClientService(1);
}


void CDlgServiceManager::OnMenuDialogServiceManagerListShowStop()
{
	// TODO: 在此添加命令处理程序代码
	ConfigClientService(2);
}
void   CDlgServiceManager::ConfigClientService(BYTE  IsMethod)
{
	DWORD  Offset = 2;
	POSITION  Position = m_CListCtrl_Dialog_Service_Manager_Show.GetFirstSelectedItemPosition();
	int  Item = m_CListCtrl_Dialog_Service_Manager_Show.GetNextSelectedItem(Position);
	CString  v1 = m_CListCtrl_Dialog_Service_Manager_Show.GetItemText(Item, 0);
	char*  ServiceName = v1.GetBuffer(0);
	LPBYTE  BufferData = (LPBYTE)LocalAlloc(LPTR, 3 + lstrlen(ServiceName));
	BufferData[0] = CLIENT_SERVICE_MANAGER_CONFIG_REQUIRE;
	BufferData[1] = IsMethod;
	memcpy(BufferData + Offset, ServiceName, lstrlen(ServiceName) + 1);
	m_IOCPServer->OnPreSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

void CDlgServiceManager::OnMenuDialogServiceManagerListShowStartAuto()
{
	// TODO: 在此添加命令处理程序代码
	ConfigClientService(3);
}


void CDlgServiceManager::OnMenuDialogServiceManagerListShowStartManual()
{
	// TODO: 在此添加命令处理程序代码
	ConfigClientService(4);
}
