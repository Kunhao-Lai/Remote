// DlgRegisterManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRegisterManager.h"
#include "afxdialogex.h"

enum  MYKEY {
	MHKEY_CLASSES_ROOT,
	MHKEY_CURRENT_USER,
	MHKEY_LOCAL_MACHINE,
	MHKEY_USERS,
	MHKEY_CURRENT_CONFIG
};
enum  KEYVALUE {
	MREG_SZ,
	MREG_DWORD,
	MREG_BINARY,
	MREG_EXPAND_SZ
};
struct  PACKET_HEADER {
	int  PathCount;      //名字个数
	DWORD  PathLength;     //名字大小
	DWORD  PathName;  //值大小
};
// CDlgRegisterManager 对话框

IMPLEMENT_DYNAMIC(CDlgRegisterManager, CDialog)

CDlgRegisterManager::CDlgRegisterManager(CWnd* pParent, _CIOCPServer* IOCPServer,
	PCONTEXT_OBJECT  ContextObject)
	: CDialog(IDD_DIALOG_REGISTER_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgRegisterManager::~CDlgRegisterManager()
{
}

void CDlgRegisterManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_DIALOG_REGISTER_MANAGER_SHOW, m_CTreeCtrl_Dialog_Register_Manager_Show);
	DDX_Control(pDX, IDC_LIST_DIALOG_REGISTER_MANAGER_SHOW, m_CListCtrl_Dialog_Register_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgRegisterManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_DIALOG_REGISTER_MANAGER_SHOW, &CDlgRegisterManager::OnTvnSelchangedTreeDialogRegisterManagerShow)
END_MESSAGE_MAP()


// CDlgRegisterManager 消息处理程序
void  CDlgRegisterManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	switch (m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(0)[0])
	{
	case  CLIENT_REGISTER_MANAGER_PATH_DATA_REPLY:
	{
		AddPath((char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1)));
		break;
	}
	case  CLIENT_REGISTER_MANAGER_KEY_DATA_REPLY:
	{
		AddKey((char*)(m_ContextObject->m_ReceivedDecompressedBufferData.GetArray(1)));
		break;
	}
	default:
		break;
	}
}


BOOL CDlgRegisterManager::OnInitDialog()
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
	v1.Format("\\\\%s - 远程注册表管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	//制作树控件上的图标
	m_ImageListTree.Create(18, 18, ILC_COLOR16, 10, 0);
	m_hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FATHER), IMAGE_ICON, 18, 18, 0);
	m_ImageListTree.Add(m_hIcon);
	m_hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FILE), IMAGE_ICON, 18, 18, 0);
	m_ImageListTree.Add(m_hIcon);

	m_CTreeCtrl_Dialog_Register_Manager_Show.SetImageList(&m_ImageListTree, TVSIL_NORMAL);

	m_RootTreeItem = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("注册表管理", 0, 0, 0, 0);
	m_CurrentUserTreeItem = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_CURRENT_USER", 1, 1, m_RootTreeItem, 0);
	m_LocalMachineTreeItem = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_LOCAL_MACHINE", 1, 1, m_RootTreeItem, 0);
	m_UsersTreeItem = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_USERS", 1, 1, m_RootTreeItem, 0);
	m_CurrentConfigTreeItem = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_CURRENT_CONFIG", 1, 1, m_RootTreeItem, 0);
	m_ClassRootTreeItem = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_CLASSES_ROOT", 1, 1, m_RootTreeItem, 0);

	m_CTreeCtrl_Dialog_Register_Manager_Show.Expand(m_RootTreeItem, TVE_EXPAND);

	m_CListCtrl_Dialog_Register_Manager_Show.InsertColumn(0, "名称", LVCFMT_LEFT, 150, -1);
	m_CListCtrl_Dialog_Register_Manager_Show.InsertColumn(1, "类型", LVCFMT_LEFT, 60, -1);
	m_CListCtrl_Dialog_Register_Manager_Show.InsertColumn(2, "数据", LVCFMT_LEFT, 300, -1);
	m_CListCtrl_Dialog_Register_Manager_Show.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	//添加图标
	m_CImageList_List.Create(16, 16, TRUE, 2, 2);
	m_CImageList_List.Add(AfxGetApp()->LoadIcon(IDI_ICON_STRING));
	m_CImageList_List.Add(AfxGetApp()->LoadIcon(IDI_ICON_DWORD));
	m_CListCtrl_Dialog_Register_Manager_Show.SetImageList(&m_CImageList_List, LVSIL_SMALL);

	m_IsEnable = TRUE;
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgRegisterManager::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_ContextObject->DialogHandle = NULL;
	m_ContextObject->DialogID = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}


void CDlgRegisterManager::OnTvnSelchangedTreeDialogRegisterManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	if (!m_IsEnable)
	{
		return;
	}
	m_IsEnable = FALSE;
	//获得你点击的Item
	TVITEM  Item = pNMTreeView->itemNew;
	if (Item.hItem == m_RootTreeItem)
	{
		m_IsEnable = TRUE;
		return;
	}
	//保存用户打开的子树节点句柄
	m_SelectedTreeItem = Item.hItem;
	m_CListCtrl_Dialog_Register_Manager_Show.DeleteAllItems();
	CString  FullPath = GetFullPath(m_SelectedTreeItem);
	char  IsToken = GetFatherPath(FullPath);

	//这个While循环是解决反复点击一个选项
	while (m_CTreeCtrl_Dialog_Register_Manager_Show.GetChildItem(Item.hItem) != NULL)
	{
		m_CTreeCtrl_Dialog_Register_Manager_Show.DeleteItem(m_CTreeCtrl_Dialog_Register_Manager_Show.GetChildItem(Item.hItem));
	}
	int  v1 = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, "(默认)", 0);
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(v1, 1, "REG_SZ");
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(v1, 2, "(数据未设置值)"); 

	FullPath.Insert(0, IsToken);
	IsToken = CLIENT_REGISTER_MANAGER_DATA;
	FullPath.Insert(0, IsToken);
	m_IOCPServer->OnPreSending(m_ContextObject, (LPBYTE)(FullPath.GetBuffer(0)), FullPath.GetLength() + 1);
	m_IsEnable = TRUE;
	*pResult = 0;
}
char    CDlgRegisterManager::GetFatherPath(CString&  SelectedTreeItem)
{
	char  IsToken = 0;
	if (!SelectedTreeItem.Find("HKEY_CLASSES_ROOT"))        //判断主键
	{
		IsToken = MHKEY_CLASSES_ROOT;
		SelectedTreeItem.Delete(0, sizeof("HKEY_CLASSES_ROOT"));
	}
	else if(!SelectedTreeItem.Find("HKEY_CURRENT_USER"))
	{
		IsToken = MHKEY_CURRENT_USER;
		SelectedTreeItem.Delete(0, sizeof("HKEY_CURRENT_USER"));
	}
	else if (!SelectedTreeItem.Find("HKEY_LOCAL_MACHINE"))
	{
		IsToken = MHKEY_LOCAL_MACHINE;
		SelectedTreeItem.Delete(0, sizeof("HKEY_LOCAL_MACHINE"));
	}
	else if (!SelectedTreeItem.Find("HKEY_USERS"))
	{
		IsToken = MHKEY_USERS;
		SelectedTreeItem.Delete(0, sizeof("HKEY_USERS"));
	}
	else if (!SelectedTreeItem.Find("HKEY_CURRENT_CONFIG"))
	{
		IsToken = MHKEY_CURRENT_CONFIG;
		SelectedTreeItem.Delete(0, sizeof("HKEY_CURRENT_CONFIG"));
	}
	return IsToken;
}
CString   CDlgRegisterManager::GetFullPath(HTREEITEM  SelectedTreeItem)
{
	CString v1;
	CString v2 = "";
	while (1)
	{
		if (SelectedTreeItem == m_RootTreeItem)
		{
			return v2;
		}
		v1 = m_CTreeCtrl_Dialog_Register_Manager_Show.GetItemText(SelectedTreeItem);
		if (v1.Right(1) != "\\")
		{
			v1 += "\\";
		}
		v2 = v1 + v2;
		SelectedTreeItem = m_CTreeCtrl_Dialog_Register_Manager_Show.GetParentItem(SelectedTreeItem);
	}
	return  v2;
}
void   CDlgRegisterManager::AddPath(char*  BufferData)
{
	if (BufferData == NULL)
	{
		return ;
	}
	int  PacketHeaderLength = sizeof(PACKET_HEADER);
	PACKET_HEADER   PacketHeader;
	memcpy((void*)&PacketHeader, BufferData, PacketHeaderLength);
	DWORD  v2 = PacketHeader.PathLength;
	int   v1 = PacketHeader.PathCount;
	if (v2 > 0 && v1 > 0)
	{
		for (int i = 0; i < v1; i++)
		{
			char*  PathName = BufferData + v2*i + PacketHeaderLength;
			m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem(PathName, 1, 1, m_SelectedTreeItem, 0);
			m_CTreeCtrl_Dialog_Register_Manager_Show.Expand(m_SelectedTreeItem, TVE_EXPAND);
		}
	}
}
void   CDlgRegisterManager::AddKey(char*  BufferData)
{
	m_CListCtrl_Dialog_Register_Manager_Show.DeleteAllItems();
	int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, "(Data)", 0);
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "REG_SZ");
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, "(NULL)");

	if (BufferData == NULL)
	{
		return;
	}
	PACKET_HEADER PacketHeader;
	memcpy((void*)&PacketHeader, BufferData, sizeof(PACKET_HEADER));
	char*  Offset = BufferData + sizeof(PACKET_HEADER);
	for (int i = 0; i < PacketHeader.PathCount; i++)
	{
		BYTE Type = Offset[0];
		Offset += sizeof(BYTE);
		char* ValueName = Offset;
		Offset += PacketHeader.PathLength;
		BYTE*  ValueData = (BYTE*)Offset;
		Offset += PacketHeader.PathName;
		if (Type == MREG_SZ)
		{
			int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, ValueName, 0);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "REG_SZ");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, (char*)ValueData);
		}
		if (Type == MREG_DWORD)
		{
			char v1[256];
			DWORD v2 = 0;
			memcpy((void*)&v2, ValueData, sizeof(DWORD));

			CString v3;
			v3.Format("0x%x", v2);
		
		
			int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, ValueName, 1);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "REG_DWORD");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, v3);
		}
		if (Type == MREG_BINARY)
		{
			char v1[256];
			sprintf(v1, "%d", ValueData);

			int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, ValueName, 1);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "REG_BINARY");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, v1);
		}
		if (Type == MREG_EXPAND_SZ)
		{
			int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, ValueName, 0);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "REG_EXPAND_SZ");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, (char*)ValueData);
		}
	}

}