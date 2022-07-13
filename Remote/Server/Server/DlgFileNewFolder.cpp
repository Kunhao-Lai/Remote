// DlgFileNewFolder.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgFileNewFolder.h"
#include "afxdialogex.h"


// CDlgFileNewFolder 对话框

IMPLEMENT_DYNAMIC(CDlgFileNewFolder, CDialog)

CDlgFileNewFolder::CDlgFileNewFolder(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_FILE_NEW_FOLDER, pParent)
	, m_CEdit_New_Folder_Main(_T(""))
{

}

CDlgFileNewFolder::~CDlgFileNewFolder()
{
}

void CDlgFileNewFolder::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DIALOG_NEW_FOLDER_MAIN, m_CEdit_New_Folder_Main);
}


BEGIN_MESSAGE_MAP(CDlgFileNewFolder, CDialog)
END_MESSAGE_MAP()


// CDlgFileNewFolder 消息处理程序
