// DlgFileManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgFileManager.h"
#include "afxdialogex.h"
#include"resource.h"
#include"DlgFileNewFolder.h"

static  UINT  __Indicators[] =
{
	ID_SEPARATOR,
	ID_SEPARATOR,
	IDR_STATUSBAR_FILE_MANAGER_PROGRESS,

};
// CDlgFileManager 对话框

IMPLEMENT_DYNAMIC(CDlgFileManager, CDialog)

CDlgFileManager::CDlgFileManager(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialog(IDD_DIALOG_FILE_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	memset(m_ClientData, 0, sizeof(m_ClientData));
	memcpy(m_ClientData, ContextObject->m_ReceivedDecompressedBufferData.GetArray(1),
		ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1);

	SHFILEINFO  v1;
	//卷图标
	HIMAGELIST  ImageListHwnd;
	ImageListHwnd = (HIMAGELIST)SHGetFileInfo
	(
		NULL,
		0,
		&v1,
		sizeof(SHFILEINFO),
		SHGFI_LARGEICON | SHGFI_SYSICONINDEX
	);
	m_CImageList_Large = CImageList::FromHandle(ImageListHwnd);
	ImageListHwnd = (HIMAGELIST)SHGetFileInfo
	(
		NULL,
		0,
		&v1,
		sizeof(SHFILEINFO),
		SHGFI_SMALLICON | SHGFI_SYSICONINDEX
	);
	m_CImageList_Small = CImageList::FromHandle(ImageListHwnd);
	m_IsDragging = FALSE;
	m_IsSTop = FALSE;
}

CDlgFileManager::~CDlgFileManager()
{
}

void CDlgFileManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DIALOG_FILE_MANAGER_SERVER_FILE, m_CListCtrl_Dialog_File_Manager_Server_File);
	DDX_Control(pDX, IDC_LIST_DIALOG_FILE_MANAGER_CLIENT_FILE, m_CListCtrl_Dialog_File_Manager_Client_File);
	DDX_Control(pDX, IDC_STATIC_DIALOG_FILE_MANAGER_SERVER_POSITION, m_CStatic_Dialog_File_Manager_Server_Position);
	DDX_Control(pDX, IDC_STATIC_DIALOG_FILE_MANAGER_CLIENT_POSITION, m_CStatic_Dialog_File_Manager_Client_Position);
	DDX_Control(pDX, IDC_COMBO_DIALOG_FILE_MANAGER_SERVER_FILE, m_CCombox_Dialog_File_Manager_Server_File);
	DDX_Control(pDX, IDC_COMBO_DIALOG_FILE_MANAGER_CLIENT_FILE, m_CCombox_Dialog_File_Manager_Client_File);
}


BEGIN_MESSAGE_MAP(CDlgFileManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DIALOG_FILE_MANAGER_SERVER_FILE, &CDlgFileManager::OnNMDblclkListDialogFileManagerServerFile)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DIALOG_FILE_MANAGER_CLIENT_FILE, &CDlgFileManager::OnNMDblclkListDialogFileManagerClientFile)
	ON_COMMAND(IDT_TOOLBAR_FILE_MANAGER_MAIN_PREVIOUS, &CDlgFileManager::OnFileManagerServerFilePrevious)
	ON_COMMAND(IDT_TOOLBAR_FILE_MANAGER_MAIN_PREVIOUS2, &CDlgFileManager::OnFileManagerClientFilePrevious)

	ON_COMMAND(IDT_TOOLBAR_FILE_MANAGER_MAIN_DELETE, &CDlgFileManager::OnFileManagerServerFileDelete)
	ON_COMMAND(IDT_TOOLBAR_FILE_MANAGER_MAIN_DELETE2, &CDlgFileManager::OnFileManagerClientFileDelete)

	ON_COMMAND(IDT_TOOLBAR_FILE_MANAGER_MAIN_NEW_FOLDER, &CDlgFileManager::OnFileManagerServerFileNewFolder)
	ON_COMMAND(IDT_TOOLBAR_FILE_MANAGER_MAIN_NEW_FOLDER2, &CDlgFileManager::OnFileManagerClientFileNewFolder)

	ON_COMMAND(IDT_TOOLBAR_FILE_MANAGER_MAIN_STOP, &CDlgFileManager::OnFileManagerServerFileStop)
	ON_COMMAND(IDT_TOOLBAR_FILE_MANAGER_MAIN_STOP2, &CDlgFileManager::OnFileManagerClientFileStop)

	ON_COMMAND(IDT_TOOLBAR_FILE_MANAGER_MAIN_VIEW, &CDlgFileManager::OnFileManagerServerFileViewSmall)
	ON_COMMAND(IDT_TOOLBAR_FILE_MANAGER_MAIN_VIEW2, &CDlgFileManager::OnFileManagerClientFileViewSmall)


END_MESSAGE_MAP()


// CDlgFileManager 消息处理程序


void CDlgFileManager::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_ContextObject->DialogHandle = NULL;
	m_ContextObject->DialogID = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}


BOOL CDlgFileManager::OnInitDialog()
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
	v1.Format("\\\\%s - 远程文件管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	//界面设置
	if (!m_ToolBar_File_Manager_Server_File.Create(this, WS_CHILD |
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, IDR_TOOLBAR_FILE_MANAGER_SERVER)
		|| !m_ToolBar_File_Manager_Server_File.LoadToolBar(IDR_TOOLBAR_FILE_MANAGER_SERVER))
	{
		return -1;
	}
	m_ToolBar_File_Manager_Server_File.LoadTrueColorToolBar
	(
		24,        //加载真彩工具条
		IDB_BITMAP_FILE_MANAGER,
		IDB_BITMAP_FILE_MANAGER,
		IDB_BITMAP_FILE_MANAGER
	);

	m_ToolBar_File_Manager_Server_File.AddDropDownButton(this, IDT_TOOLBAR_FILE_MANAGER_MAIN_VIEW, IDT_TOOLBAR_FILE_MANAGER_MAIN_VIEW);
	m_ToolBar_File_Manager_Server_File.SetButtonText(0, "返回");
	m_ToolBar_File_Manager_Server_File.SetButtonText(1, "查看");
	m_ToolBar_File_Manager_Server_File.SetButtonText(2, "删除");
	m_ToolBar_File_Manager_Server_File.SetButtonText(3, "新建");
	m_ToolBar_File_Manager_Server_File.SetButtonText(4, "查找");
	m_ToolBar_File_Manager_Server_File.SetButtonText(5, "停止");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);        //显示
	m_CListCtrl_Dialog_File_Manager_Server_File.SetImageList(m_CImageList_Large, LVSIL_NORMAL);
	m_CListCtrl_Dialog_File_Manager_Server_File.SetImageList(m_CImageList_Small, LVSIL_SMALL);

	if (!m_ToolBar_File_Manager_Client_File.Create(this, WS_CHILD |
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, IDR_TOOLBAR_FILE_MANAGER_CLIENT)
		|| !m_ToolBar_File_Manager_Client_File.LoadToolBar(IDR_TOOLBAR_FILE_MANAGER_CLIENT))
	{
		return -1;
	}
	m_ToolBar_File_Manager_Client_File.LoadTrueColorToolBar
	(
		24,        //加载真彩工具条
		IDB_BITMAP_FILE_MANAGER,
		IDB_BITMAP_FILE_MANAGER,
		IDB_BITMAP_FILE_MANAGER
	);

	m_ToolBar_File_Manager_Client_File.AddDropDownButton(this, IDT_TOOLBAR_FILE_MANAGER_MAIN_VIEW2, IDT_TOOLBAR_FILE_MANAGER_MAIN_VIEW2);
	m_ToolBar_File_Manager_Client_File.SetButtonText(0, "返回");
	m_ToolBar_File_Manager_Client_File.SetButtonText(1, "查看");
	m_ToolBar_File_Manager_Client_File.SetButtonText(2, "删除");
	m_ToolBar_File_Manager_Client_File.SetButtonText(3, "新建");
	m_ToolBar_File_Manager_Client_File.SetButtonText(4, "查找");
	m_ToolBar_File_Manager_Client_File.SetButtonText(5, "停止");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);        //显示
	m_CListCtrl_Dialog_File_Manager_Client_File.SetImageList(m_CImageList_Large, LVSIL_NORMAL);
	m_CListCtrl_Dialog_File_Manager_Client_File.SetImageList(m_CImageList_Small, LVSIL_SMALL);

	RECT  ClientRect;
	GetClientRect(&ClientRect);

	CRect  v3;
	v3.top = ClientRect.bottom - 25;
	v3.left = 0;
	v3.right = ClientRect.right;
	v3.bottom = ClientRect.bottom;
	if (!m_StatusBar.Create(this) || !m_StatusBar.SetIndicators(__Indicators, sizeof(__Indicators) / sizeof(UINT)))
	{
		return  -1;
	}
	m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_StatusBar.SetPaneInfo(1, m_StatusBar.GetItemID(1), SBPS_NORMAL, 120);
	m_StatusBar.SetPaneInfo(2, m_StatusBar.GetItemID(2), SBPS_NORMAL, 50);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);        //显示状态栏

	m_StatusBar.MoveWindow(v3);
	m_StatusBar.GetItemRect(1, &ClientRect);
	ClientRect.bottom -= 1;
	m_ProgressCtrl = new  CProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, ClientRect, &m_StatusBar, 1);
	m_ProgressCtrl->SetRange(0, 100);
	m_ProgressCtrl->SetPos(0);

	//Serevr真彩
	RECT  ServerRect;
	m_CStatic_Dialog_File_Manager_Server_Position.GetWindowRect(&ServerRect);
	m_CStatic_Dialog_File_Manager_Server_Position.ShowWindow(SW_HIDE);
	//显示工具栏
	m_ToolBar_File_Manager_Server_File.MoveWindow(&ServerRect);
	m_CStatic_Dialog_File_Manager_Client_Position.GetWindowRect(&ServerRect);
	m_CStatic_Dialog_File_Manager_Client_Position.ShowWindow(SW_HIDE);
	//显示工具栏
	m_ToolBar_File_Manager_Client_File.MoveWindow(&ServerRect);
	//Client真彩


	FixedServerVolumeData();
	FixedClientVolumeData();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
void  CDlgFileManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case  CLIENT_FILE_MANAGER_FILE_DATA_REPLY:
	{
		FixedClientFileData(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(),
			m_ContextObject->m_ReceivedDecompressedBufferData.GetArrayLength() - 1);
		break;
	}

	}
}
void  CDlgFileManager::FixedClientFileData(BYTE*  BufferData, ULONG  BufferLength)
{
	
	m_CListCtrl_Dialog_File_Manager_Client_File.DeleteAllItems();

	while (m_CListCtrl_Dialog_File_Manager_Client_File.DeleteColumn(0) != 0);
	//初始化列表信息
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(0, "名称", LVCFMT_CENTER, 150);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(1, "大小", LVCFMT_CENTER, 100);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(2, "类型", LVCFMT_CENTER, 100);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(3, "修改日期", LVCFMT_CENTER, 110);
	int  v10 = 0;
	//自己在ControlList的控件上写入一个..目录（一旦双击就返回上一层）
	m_CListCtrl_Dialog_File_Manager_Client_File.SetItemData(m_CListCtrl_Dialog_File_Manager_Client_File.InsertItem(v10++,
		"..", GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);
	if (BufferLength != 0)
	{
		for (int i = 0; i < 2; i++)
		{
			char* Travel = (char*)(BufferData + 1);
			for (char* v1 = Travel; Travel - v1 < BufferLength - 1;)
			{
				char*  FileName = NULL;
				DWORD  FileSizeHigh = 0;       //文件高字节大小
				DWORD  FileSizeLow = 0;        //文件低字节大小
				int    Item = 0;
				BOOL   IsInsert = FALSE;
				FILETIME   FileTime;
				int  v3 = *Travel ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				IsInsert = !(v3 == FILE_ATTRIBUTE_DIRECTORY) == i;

				FileName = ++Travel;
				if (IsInsert)
				{
					Item = m_CListCtrl_Dialog_File_Manager_Client_File.InsertItem(v10++, FileName, GetIconIndex(FileName, v3));
					m_CListCtrl_Dialog_File_Manager_Client_File.SetItemData(Item, v3 == FILE_ATTRIBUTE_DIRECTORY);
					SHFILEINFO  v1;
					SHGetFileInfo(FileName, FILE_ATTRIBUTE_NORMAL | v3, &v1,
						sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
					m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 2, v1.szTypeName);
				}
				//得到文件大小
				Travel += strlen(FileName) + 1;
				if (IsInsert)
				{
					memcpy(&FileSizeHigh, Travel, 4);
					memcpy(&FileSizeLow, Travel + 4, 4);
					CString v7;
					v7.Format("%10d KB", (FileSizeHigh*(MAXDWORD + 1)) / 1024 + FileSizeLow / 1024 + (FileSizeLow % 1024 ? 1 : 0));
					m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 1, v7);
					memcpy(&FileTime, Travel + 8, sizeof(FILETIME));
					CTime  Time(FileTime);
					m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 3, Time.Format("%Y-%m-%d  %H:%M"));
				}
				Travel += 16;
			}
		}
	}
	//恢复窗口
	m_CListCtrl_Dialog_File_Manager_Client_File.EnableWindow(TRUE);
}
void  CDlgFileManager::FixedServerVolumeData()
{
	char  VolumeData[0x500] = { 0 };
	char*  Travel = NULL;
	m_CListCtrl_Dialog_File_Manager_Server_File.DeleteAllItems();

	while (m_CListCtrl_Dialog_File_Manager_Server_File.DeleteColumn(0) != 0);
	//初始化列表信息
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(0, "名称", LVCFMT_CENTER, 40);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(1, "类型", LVCFMT_CENTER, 60);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(2, "文件系统", LVCFMT_CENTER, 65);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(3, "总大小", LVCFMT_CENTER, 80);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(4, "可用空间", LVCFMT_CENTER, 80);

	m_CListCtrl_Dialog_File_Manager_Server_File.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	//获得驱动器信息
	GetLogicalDriveStrings(sizeof(VolumeData), VolumeData);

	Travel = VolumeData;
	CHAR  FileSystem[MAX_PATH];
	unsigned __int64    HardDiskAmount = 0;
	unsigned __int64    HardDiskFreeSpace = 0;
	unsigned  long      HardDiskAmountMB = 0;
	unsigned  long      HardDiskFreeSpaceMB = 0;
	for (int i = 0; *Travel != '\0';i++, Travel += lstrlen(Travel) + 1)
	{
		memset(FileSystem, 0, sizeof(FileSystem));
		//得到文件系统信息及大小
		GetVolumeInformation(Travel, NULL, 0, NULL, NULL, NULL, FileSystem, MAX_PATH);
		ULONG  FileSystemLength = lstrlen(FileSystem) + 1;
		
		//计算磁盘大小
		if (GetDiskFreeSpaceEx(Travel, (PULARGE_INTEGER)&HardDiskFreeSpace,
			(PULARGE_INTEGER)&HardDiskAmount, NULL))
		{
			HardDiskAmountMB = HardDiskAmount / 1024 / 1024;
			HardDiskFreeSpaceMB = HardDiskFreeSpace / 1024 / 1024;
		}
		else
		{
			HardDiskAmountMB = 0;
			HardDiskFreeSpaceMB = 0;
		}
		int  Item = m_CListCtrl_Dialog_File_Manager_Server_File.InsertItem(i, Travel,
			GetIconIndex(Travel, GetFileAttributes(Travel)));      //获得系统图标
		m_CListCtrl_Dialog_File_Manager_Server_File.SetItemData(Item, 1);
		m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 2, FileSystem);

		SHFILEINFO  v1;
		SHGetFileInfo(Travel, FILE_ATTRIBUTE_NORMAL, &v1,
			sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 1, v1.szTypeName);

		CString  v2;
		v2.Format("%10.1f GB", (float)HardDiskAmountMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 3, v2);
		v2.Format("%10.1f GB", (float)HardDiskFreeSpaceMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 4, v2);
	}
}
void  CDlgFileManager::FixedClientVolumeData()
{
	m_CListCtrl_Dialog_File_Manager_Client_File.DeleteAllItems();
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(0, "名称", LVCFMT_CENTER, 40);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(1, "类型", LVCFMT_CENTER, 60);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(2, "文件系统", LVCFMT_CENTER, 65);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(3, "总大小", LVCFMT_CENTER, 80);
	m_CListCtrl_Dialog_File_Manager_Client_File.InsertColumn(4, "可用空间", LVCFMT_CENTER, 80);
	m_CListCtrl_Dialog_File_Manager_Client_File.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	char*  Travel = NULL;
	Travel = (char*)m_ClientData;
	int i = 0;
	ULONG  v1 = 0;
	for (i = 0; Travel[i] != '\0';)
	{
		//由驱动器名判断图标的索引
		if (Travel[i] == 'A' || Travel[i] == 'B')
		{
			v1 = 6;
		}
		else
		{
			switch (Travel[i + 1])         //这里判断驱动类型
			{
			case DRIVE_REMOVABLE:
				v1 = 2 + 5;
				break;
			case  DRIVE_FIXED:
				v1 = 3 + 5;
				break;
			case  DRIVE_REMOTE:
				v1 = 4 + 5;
				break;
			case  DRIVE_CDROM:
				v1 = 9;
				break;
			default:
				v1 = 0;
				break;
			}
		}
		CString v2;
		//格式化盘符
		v2.Format("%c:\\", Travel[i]);
		int  Item = m_CListCtrl_Dialog_File_Manager_Client_File.InsertItem(i, v2, v1);
		m_CListCtrl_Dialog_File_Manager_Client_File.SetItemData(Item, 1);
		unsigned  long      HardDiskAmountMB = 0;
		unsigned  long      HardDiskFreeSpaceMB = 0;
		memcpy(&HardDiskAmountMB, Travel + i + 2, 4);
		memcpy(&HardDiskFreeSpaceMB, Travel + i + 6, 4);
		v2.Format("%10.1f GB", (float)HardDiskAmountMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 3, v2);
		v2.Format("%10.1f GB", (float)HardDiskFreeSpaceMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 4, v2);
		
		i += 10;
		CString		v7;
		v7 = Travel + i;
		m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 1, v7);
		i += strlen(Travel + i) + 1;
		CString  FileSystem;
		FileSystem = Travel + i;
		m_CListCtrl_Dialog_File_Manager_Client_File.SetItemText(Item, 2, FileSystem);
		i += strlen(Travel + i) + 1;
	}
}
int   CDlgFileManager::GetIconIndex(LPCTSTR  VolumeName, DWORD  FileAttributes)
{
	SHFILEINFO  v1;
	if (FileAttributes == INVALID_FILE_ATTRIBUTES)
		FileAttributes = FILE_ATTRIBUTE_NORMAL;
	else
		FileAttributes |= FILE_ATTRIBUTE_NORMAL;


	SHGetFileInfo(VolumeName, FileAttributes, &v1,
		sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
	return  v1.iIcon;
}
void CDlgFileManager::OnNMDblclkListDialogFileManagerServerFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	if (m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectedCount() == 0 ||
		m_CListCtrl_Dialog_File_Manager_Server_File.GetItemData(m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectionMark()) != 1)
	{
		return;
	}
	FixedServerFileData();
	*pResult = 0;
}
void  CDlgFileManager::FixedServerFileData(CString  Directory)
{
	if (Directory.GetLength() == 0)
	{
		int  Item = m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectionMark();
		if (Item != -1)
		{
			if (m_CListCtrl_Dialog_File_Manager_Server_File.GetItemData(Item) == 1)
			{
				Directory = m_CListCtrl_Dialog_File_Manager_Server_File.GetItemText(Item, 0);
			}
		}
		else
		{
			//
		}
	}
	if (Directory == "..")
	{
		//返回上一层目录
		m_ServerFileFullPath = GetParentDirectory(m_ServerFileFullPath);
	}
	else if (Directory != ".")
	{
		m_ServerFileFullPath += Directory;
		if (m_ServerFileFullPath.Right(1) != "\\")
		{
			m_ServerFileFullPath += "\\";
		}
	}
	if (m_ServerFileFullPath.GetLength() == 0)
	{
		FixedServerVolumeData();
		return;
	}
	m_CCombox_Dialog_File_Manager_Server_File.InsertString(0, m_ServerFileFullPath);
	m_CCombox_Dialog_File_Manager_Server_File.SetCurSel(0);
	m_CListCtrl_Dialog_File_Manager_Server_File.DeleteAllItems();

	while (m_CListCtrl_Dialog_File_Manager_Server_File.DeleteColumn(0) != 0);
	//初始化列表信息
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(0, "名称", LVCFMT_CENTER, 150);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(1, "大小", LVCFMT_CENTER, 100);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(2, "类型", LVCFMT_CENTER, 100);
	m_CListCtrl_Dialog_File_Manager_Server_File.InsertColumn(3, "修改日期", LVCFMT_CENTER, 110);
	int  v10 = 0;
	//自己在ControlList的控件上写入一个..目录（一旦双击就返回上一层）
	m_CListCtrl_Dialog_File_Manager_Server_File.SetItemData(m_CListCtrl_Dialog_File_Manager_Server_File.InsertItem(v10++,
		"..", GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);
	for (int i = 0; i < 2; i++)
	{
		CFileFind  FileFind;
		BOOL      IsLoop;
		IsLoop = FileFind.FindFile(m_ServerFileFullPath + "*.*");
		while (IsLoop)
		{
			IsLoop = FileFind.FindNextFile();
			if (FileFind.IsDots())
				continue;
			BOOL   IsInsert = !FileFind.IsDirectory() == i;
			if (!IsInsert)
				continue;
			int  Item = m_CListCtrl_Dialog_File_Manager_Server_File.InsertItem(v10++, FileFind.GetFileName(),
				GetIconIndex(FileFind.GetFileName(), GetFileAttributes(FileFind.GetFilePath())));
			//如果是目录设置隐藏数据为1
			m_CListCtrl_Dialog_File_Manager_Server_File.SetItemData(Item, FileFind.IsDirectory());
			SHFILEINFO  v1;
			SHGetFileInfo(FileFind.GetFileName(), FILE_ATTRIBUTE_NORMAL, &v1,
				sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
			if (FileFind.IsDirectory())
			{
				m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 2, "文件夹");
			}
			else
			{
				m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 2, v1.szTypeName);
			}
			CString v2;
			v2.Format("%10d KB", FileFind.GetLength() / 1024 + (FileFind.GetLength() % 1024 ? 1 : 0));
			m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 1, v2);
			CTime  Time;
			FileFind.GetLastWriteTime(Time);
			m_CListCtrl_Dialog_File_Manager_Server_File.SetItemText(Item, 3, Time.Format("%Y-%m-%d  %H:%M"));
		}
	}

}
CString  CDlgFileManager::GetParentDirectory(CString	  FileFullPath)
{
	CString v1 = FileFullPath;
	int  Index = v1.ReverseFind('\\');
	if (Index == -1)
	{
		return v1;
	}
	CString  v3 = v1.Left(Index);
	Index = v3.ReverseFind('\\');
	if (Index == -1)
	{
		v1 = "";
		return v1;
	}
	v1 = v3.Left(Index);
	if (v1.Right(1) != "\\")
	{
		v1 += "\\";
	}
	return  v1;
}

void CDlgFileManager::OnNMDblclkListDialogFileManagerClientFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	if (m_CListCtrl_Dialog_File_Manager_Client_File.GetSelectedCount() == 0 ||
		m_CListCtrl_Dialog_File_Manager_Client_File.GetItemData(m_CListCtrl_Dialog_File_Manager_Client_File.GetSelectionMark()) != 1)
	{
		return;
	}
	//向客户端发消息
	GetClientFileData();
	*pResult = 0;
}
void  CDlgFileManager::GetClientFileData(CString  Directory)
{
	if (Directory.GetLength() == 0)
	{
		int  Item = m_CListCtrl_Dialog_File_Manager_Client_File.GetSelectionMark();
		if (Item != -1)
		{
			if (m_CListCtrl_Dialog_File_Manager_Client_File.GetItemData(Item) == 1)
			{
				Directory = m_CListCtrl_Dialog_File_Manager_Client_File.GetItemText(Item, 0);
			}
		}
		else
		{
			//
		}
	}
	if (Directory == "..")
	{
		//返回上一层目录
		m_ClientFileFullPath = GetParentDirectory(m_ClientFileFullPath);
	}
	else if (Directory != ".")
	{
		m_ClientFileFullPath += Directory;
		if (m_ClientFileFullPath.Right(1) != "\\")
		{
			m_ClientFileFullPath += "\\";
		}
	}
	if (m_ClientFileFullPath.GetLength() == 0)
	{
		FixedClientVolumeData();
		return;
	}
	ULONG  BufferLength = m_ClientFileFullPath.GetLength() + 2;
	BYTE*  BufferData = (BYTE*)new  BYTE[BufferLength];
	BufferData[0] = CLIENT_FILE_MANAGER_FILE_DATA_REQUIRE;
	memcpy(BufferData + 1, m_ClientFileFullPath.GetBuffer(0), BufferLength - 1);
	m_IOCPServer->OnPreSending(m_ContextObject, BufferData, BufferLength);
	delete[]  BufferData;
	BufferData = NULL;

	m_CListCtrl_Dialog_File_Manager_Client_File.EnableWindow(FALSE);
	m_ProgressCtrl->SetPos(0);
}

BOOL CDlgFileManager::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	//ComboBox回车键
	CEdit* Edit = (CEdit*)m_CCombox_Dialog_File_Manager_Server_File.GetWindow(GW_CHILD);

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && pMsg->hwnd == Edit->m_hWnd)
	{
		CString v1;
		m_CCombox_Dialog_File_Manager_Server_File.GetWindowTextA(v1);
		m_ServerFileFullPath = v1;
		FixedServerFileData(NULL);
		return FALSE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
void CDlgFileManager::EnableControl(BOOL IsEnable)
{
	m_CListCtrl_Dialog_File_Manager_Client_File.EnableWindow(IsEnable);
	m_CListCtrl_Dialog_File_Manager_Server_File.EnableWindow(IsEnable);
	m_CCombox_Dialog_File_Manager_Server_File.EnableWindow(IsEnable);
	m_CCombox_Dialog_File_Manager_Client_File.EnableWindow(IsEnable);
}
BOOL CDlgFileManager::DeleteDirectory(LPCTSTR DirectoryFullPath)
{
	WIN32_FIND_DATA	v1;
	char	BufferData[MAX_PATH] = { 0 };

	wsprintf(BufferData, "%s\\*.*", DirectoryFullPath);

	HANDLE FileHandle = FindFirstFile(BufferData, &v1);
	if (FileHandle == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败
		return FALSE;

	do
	{
		if (v1.cFileName[0] == '.'&&strlen(v1.cFileName) <= 2)
		{
			continue;
		}
		else
		{
			if (v1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char v2[MAX_PATH];
				wsprintf(v2, "%s\\%s", DirectoryFullPath, v1.cFileName);
				DeleteDirectory(v2);
			}
			else
			{
				char v2[MAX_PATH];
				wsprintf(v2, "%s\\%s", DirectoryFullPath, v1.cFileName);
				DeleteFile(v2);
			}
		}
	} while (FindNextFile(FileHandle, &v1));

	FindClose(FileHandle); // 关闭查找句柄

	if (!RemoveDirectory(DirectoryFullPath))
	{
		return FALSE;
	}
	return TRUE;
}

void CDlgFileManager::OnFileManagerServerFilePrevious()
{
	FixedServerFileData("..");
}

void CDlgFileManager::OnFileManagerServerFileDelete()
{
	CString v1;
	if (m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectedCount() > 1)
		v1.Format("确定要将这 %d 项删除吗?", m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectedCount());
	else
	{

		int Item = m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectionMark();   //.. fff 1  Hello

		if (Item == -1)
		{
			return;
		}
		CString FileName = m_CListCtrl_Dialog_File_Manager_Server_File.GetItemText(m_CListCtrl_Dialog_File_Manager_Server_File.GetSelectionMark(), 0);

		//删除的是不是目录
		if (m_CListCtrl_Dialog_File_Manager_Server_File.GetItemData(Item) == 1)
		{
			v1.Format("确实要删除文件夹“%s”并将所有内容删除吗?", FileName);
		}
		else
		{
			v1.Format("确实要把“%s”删除吗?", FileName);
		}
	}
	if (::MessageBox(m_hWnd, v1, "确认删除", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	//不让用户乱点
	EnableControl(FALSE);

	POSITION Position = m_CListCtrl_Dialog_File_Manager_Server_File.GetFirstSelectedItemPosition();
	while (Position)
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Server_File.GetNextSelectedItem(Position);
		CString	FileFullPath = m_ServerFileFullPath + m_CListCtrl_Dialog_File_Manager_Server_File.GetItemText(iItem, 0);
		// 如果是目录
		if (m_CListCtrl_Dialog_File_Manager_Server_File.GetItemData(iItem))
		{
			FileFullPath += '\\';
			DeleteDirectory(FileFullPath);
		}
		else
		{
			DeleteFile(FileFullPath);
		}
	}
	// 禁用文件管理窗口
	EnableControl(TRUE);
	FixedServerFileData(".");
}

void CDlgFileManager::OnFileManagerServerFileNewFolder()
{
	if (m_ServerFileFullPath == "")
		return;


	CDlgFileNewFolder	Dialog(this);
	if (Dialog.DoModal() == IDOK && Dialog.m_CEdit_New_Folder_Main.GetLength())
	{
		// 创建多层目录
		CString  v1;
		v1 = m_ServerFileFullPath + Dialog.m_CEdit_New_Folder_Main + "\\";
		MakeSureDirectoryPathExists(v1.GetBuffer());  /*c:\Hello\  */
		FixedServerFileData(".");
	}
}
BOOL CDlgFileManager::MakeSureDirectoryPathExists(char* DirectoryFullPath)
{

	char* Travel = NULL;
	char* BufferData = NULL;
	DWORD DirectoryAttributes;
	__try
	{
		BufferData = (char*)malloc(sizeof(char)*(strlen(DirectoryFullPath) + 1));

		if (BufferData == NULL)
		{
			return FALSE;
		}

		strcpy(BufferData, DirectoryFullPath);

		Travel = BufferData;


		if (*(Travel + 1) == ':')
		{
			Travel++;
			Travel++;
			if (*Travel && (*Travel == '\\'))
			{
				Travel++;
			}
		}
		while (*Travel)           //D:\Hello\\World\Shit\0
		{
			if (*Travel == '\\')
			{
				*Travel = '\0';
				DirectoryAttributes = GetFileAttributes(BufferData);   //查看是否是否目录  目录存在吗
				if (DirectoryAttributes == 0xffffffff)
				{
					if (!CreateDirectory(BufferData, NULL))
					{
						if (GetLastError() != ERROR_ALREADY_EXISTS)
						{
							free(BufferData);
							return FALSE;
						}
					}
				}
				else
				{
					if ((DirectoryAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					{
						free(BufferData);
						BufferData = NULL;
						return FALSE;
					}
				}

				*Travel = '\\';
			}

			Travel = CharNext(Travel);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (BufferData != NULL)
		{
			free(BufferData);

			BufferData = NULL;
		}

		return FALSE;
	}

	if (BufferData != NULL)
	{
		free(BufferData);
		BufferData = NULL;
	}
	return TRUE;
}

void CDlgFileManager::OnFileManagerServerFileStop()
{
}

void CDlgFileManager::OnFileManagerServerFileViewSmall()
{
}

void CDlgFileManager::OnFileManagerServerFileViewList()
{
}

void CDlgFileManager::OnFileManagerServerFileViewDetail()
{
}

void CDlgFileManager::OnFileManagerClientFilePrevious()
{
	GetClientFileData("..");
}

void CDlgFileManager::OnFileManagerClientFileDelete()
{
	CString v1;
	if (m_CListCtrl_Dialog_File_Manager_Client_File.GetSelectedCount() > 1)
	{
		v1.Format("确定要将这 %d 项删除吗?", m_CListCtrl_Dialog_File_Manager_Client_File.GetSelectedCount());
	}
	else
	{
		int Item = m_CListCtrl_Dialog_File_Manager_Client_File.GetSelectionMark();   //.. fff 1  Hello
		if (Item == -1)
		{
			return;
		}

		CString FileName = m_CListCtrl_Dialog_File_Manager_Client_File.GetItemText(m_CListCtrl_Dialog_File_Manager_Client_File.GetSelectionMark(), 0);
		//通过隐藏数据判断是否为文件夹
		if (m_CListCtrl_Dialog_File_Manager_Client_File.GetItemData(Item) == 1)
		{
			v1.Format("确实要删除文件夹“%s”并将所有内容删除吗?", FileName);
		}
		else
		{
			v1.Format("确实要把“%s”删除吗?", FileName);
		}
	}
	if (::MessageBox(m_hWnd, v1, "确认删除？", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}

	//禁止用户操作
	EnableControl(FALSE);
	BOOL IsDirectory = FALSE;

	POSITION Position = m_CListCtrl_Dialog_File_Manager_Client_File.GetFirstSelectedItemPosition();
	while (Position)
	{
		int Item = m_CListCtrl_Dialog_File_Manager_Client_File.GetNextSelectedItem(Position);
		CString	FileFullPath = m_ClientFileFullPath + m_CListCtrl_Dialog_File_Manager_Client_File.GetItemText(Item, 0);
		// 如果是目录
		if (m_CListCtrl_Dialog_File_Manager_Client_File.GetItemData(Item))
		{
			FileFullPath += '\\';
			IsDirectory = TRUE;
		}

		ULONG BufferLength = m_ClientFileFullPath.GetLength() + 3;
		BYTE* BufferData = new BYTE[BufferLength];

		BufferData[0] = CLIENT_FILE_MANAGER_DELETE_FILE_REQUEST;
		memcpy(BufferData + 1, &IsDirectory, sizeof(BOOL));
		memcpy(BufferData + 2, m_ClientFileFullPath.GetBuffer(0), BufferLength - 1);
		m_IOCPServer->OnPreSending(m_ContextObject, BufferData, BufferLength);

		delete[] BufferData;
		BufferData = NULL;
	}
	// 禁用文件管理窗口
	EnableControl(TRUE);
	GetClientFileData(".");

}

void CDlgFileManager::OnFileManagerClientFileNewFolder()
{
	if (m_ClientFileFullPath == "")
	{
		return;
	}
	CDlgFileNewFolder FileNewFolderDialog(this);
	if (FileNewFolderDialog.DoModal() == IDOK && FileNewFolderDialog.m_CEdit_New_Folder_Main.GetLength())
	{
		// 创建多层目录
		CString  v1;
		v1 = m_ClientFileFullPath + FileNewFolderDialog.m_CEdit_New_Folder_Main + "\\";

		ULONG	BufferLength = v1.GetLength() + 2;
		BYTE*   BufferData = (BYTE *)new BYTE[BufferLength];

		BufferData[0] = CLIENT_FILE_MANAGER_NEW_FOLDER_REQUEST;
		memcpy(BufferData + 1, v1.GetBuffer(0), BufferLength - 1);
		m_IOCPServer->OnPreSending(m_ContextObject, BufferData, BufferLength);

		delete[] BufferData;
		BufferData = NULL;

		GetClientFileData(".");
	}

}

void CDlgFileManager::OnFileManagerClientFileStop()
{
}

void CDlgFileManager::OnFileManagerClientFileViewSmall()
{
}

void CDlgFileManager::OnFileManagerClientFileViewList()
{
}

void CDlgFileManager::OnFileManagerClientFileViewDetail()
{
}
