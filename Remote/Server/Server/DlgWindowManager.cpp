// DlgWindowManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgWindowManager.h"
#include "afxdialogex.h"


// CDlgWindowManager 对话框

IMPLEMENT_DYNAMIC(CDlgWindowManager, CDialog)

CDlgWindowManager::CDlgWindowManager(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialog(IDD_DIALOG_WINDOW_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgWindowManager::~CDlgWindowManager()
{
}

void CDlgWindowManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DIALOG_WINDOW_MANAGER_SHOW, m_CListCtrl_Dialog_Window_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgWindowManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_DIALOG_WINDOW_MANAGER_SHOW, &CDlgWindowManager::OnNMRClickListDialogWindowManagerShow)
	ON_COMMAND(ID_MENU_DIALOG_WINDOW_MANAGER_LIST_SHOW_REFRESH, &CDlgWindowManager::OnMenuDialogWindowManagerListShowRefresh)
END_MESSAGE_MAP()


// CDlgWindowManager 消息处理程序


void CDlgWindowManager::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_ContextObject->DialogHandle = NULL;
	m_ContextObject->DialogID = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}


BOOL CDlgWindowManager::OnInitDialog()
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
	v1.Format("\\\\%s - 远程窗口管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);
	//设置对话框上的数据
	LOGFONT  Logfont;
	CFont*  v2 = m_CListCtrl_Dialog_Window_Manager_Show.GetFont();
	v2->GetLogFont(&Logfont);
	Logfont.lfHeight = Logfont.lfHeight*1.3;      //此处可以修改字体的高比例
	Logfont.lfWidth = Logfont.lfWidth*1.3;        //此处可以修改字体的宽比例
	CFont  v4;
	v4.CreateFontIndirect(&Logfont);
	m_CListCtrl_Dialog_Window_Manager_Show.SetFont(&v4);
	v4.Detach();

	//被控端传回的数据
	char*  BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0));
	m_CListCtrl_Dialog_Window_Manager_Show.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	switch (BufferData[0])
	{
	case  CLIENT_WINDOW_MANAGER_REPLY:
	{
		m_CListCtrl_Dialog_Window_Manager_Show.InsertColumn(0, "窗口句柄", LVCFMT_LEFT, 70);
		m_CListCtrl_Dialog_Window_Manager_Show.InsertColumn(1, "窗口名称", LVCFMT_LEFT, 300);
		m_CListCtrl_Dialog_Window_Manager_Show.InsertColumn(2, "窗口状态", LVCFMT_LEFT, 50);
		ShowWindowList();
		break;
	}
	default:
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
void  CDlgWindowManager::ShowWindowList(void)
{
	char*  BufferData = (char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1));
	DWORD  Offset = 0;
	char*  WindowTitleName = NULL;
	bool   bDel = FALSE;

	m_CListCtrl_Dialog_Window_Manager_Show.DeleteAllItems();
	CString  v1;
	int i;
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1; i++)
	{
		LPDWORD  Hwnd = LPDWORD(BufferData + Offset);            //窗口句柄
		WindowTitleName = (char*)BufferData + Offset + sizeof(HWND);         //窗口标题
		v1.Format("%5u", *Hwnd);
		m_CListCtrl_Dialog_Window_Manager_Show.InsertItem(i, v1);
		m_CListCtrl_Dialog_Window_Manager_Show.SetItemText(i, 1, WindowTitleName);
		m_CListCtrl_Dialog_Window_Manager_Show.SetItemText(i, 2, "显示");
		m_CListCtrl_Dialog_Window_Manager_Show.SetItemData(i, *Hwnd);
		Offset += sizeof(DWORD) + lstrlen(WindowTitleName) + 1;

	}
	v1.Format("窗口名称  窗口个数【%d】", i);
	LVCOLUMN  v3;
	v3.mask = LVCF_TEXT;
	v3.pszText = v1.GetBuffer(0);
	v3.cchTextMax = v1.GetLength();
	m_CListCtrl_Dialog_Window_Manager_Show.SetColumn(2, &v3);
}
void  CDlgWindowManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case  CLIENT_WINDOW_MANAGER_REPLY:
	{
		ShowWindowList();
		break;
	}
	
	}
}

void CDlgWindowManager::OnNMRClickListDialogWindowManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu   Menu;
	//加载菜单资源
	Menu.LoadMenu(IDC_LIST_DIALOG_WINDOW_MANAGER_SHOW);
	//获得鼠标位置
	CPoint  Point;
	GetCursorPos(&Point);
	SetForegroundWindow();           //设置当前窗口
	Menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
		Point.x, Point.y, this, NULL);
	*pResult = 0;
}


void CDlgWindowManager::OnMenuDialogWindowManagerListShowRefresh()
{
	// TODO: 在此添加命令处理程序代码
	BYTE  IsToken = CLIENT_WINDOW_REFRESH_REQUIRE;
	m_IOCPServer->OnPreSending(m_ContextObject, &IsToken, 1);
}
