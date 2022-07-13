// DlgTransferMode.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgTransferMode.h"
#include "afxdialogex.h"
#include "Resource.h"

// CDlgTransferMode 对话框

IMPLEMENT_DYNAMIC(CDlgTransferMode, CDialog)

CDlgTransferMode::CDlgTransferMode(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_TRANSFER_MODE, pParent)
	, m_CStatic_Transfer_Mode_Tips(_T(""))
{

}

CDlgTransferMode::~CDlgTransferMode()
{
}

void CDlgTransferMode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_TRANSFER_MODE_TIPS, m_CStatic_Transfer_Mode_Tips);
}


BEGIN_MESSAGE_MAP(CDlgTransferMode, CDialog)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_TRANSFER_MODE_OVERWRITE, IDC_BUTTON_TRANSFER_MODE_ALL_JUMP, OnEndDialog)
END_MESSAGE_MAP()


// CDlgTransferMode 消息处理程序
VOID CDlgTransferMode::OnEndDialog(UINT Key)
{
	// TODO: Add your control notification handler code here
	EndDialog(Key);
}

BOOL CDlgTransferMode::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CString	Tips;
	Tips.Format("衰了 咋办.... \" %s \" ", m_CStatic_Transfer_Mode_Tips);

	for (int i = 0; i < Tips.GetLength(); i += 120)
	{
		Tips.Insert(i, "\n");
		i += 1;
	}
	SetDlgItemText(IDC_STATIC_TRANSFER_MODE_TIPS, Tips);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
