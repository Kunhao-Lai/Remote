// DlgFileNewFolder.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgFileNewFolder.h"
#include "afxdialogex.h"


// CDlgFileNewFolder �Ի���

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


// CDlgFileNewFolder ��Ϣ�������
