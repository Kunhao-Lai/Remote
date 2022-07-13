// DlgTransferMode.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgTransferMode.h"
#include "afxdialogex.h"
#include "Resource.h"

// CDlgTransferMode �Ի���

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


// CDlgTransferMode ��Ϣ�������
VOID CDlgTransferMode::OnEndDialog(UINT Key)
{
	// TODO: Add your control notification handler code here
	EndDialog(Key);
}

BOOL CDlgTransferMode::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	CString	Tips;
	Tips.Format("˥�� զ��.... \" %s \" ", m_CStatic_Transfer_Mode_Tips);

	for (int i = 0; i < Tips.GetLength(); i += 120)
	{
		Tips.Insert(i, "\n");
		i += 1;
	}
	SetDlgItemText(IDC_STATIC_TRANSFER_MODE_TIPS, Tips);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}
