#pragma once
#include "ConfigFile.h"
#include "afxwin.h"

// CDlgServerSet �Ի���

class CDlgServerSet : public CDialog
{
	DECLARE_DYNAMIC(CDlgServerSet)

public:
	CDlgServerSet(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgServerSet();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SERVER_SET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	long m_CEdit_Server_Set_MaxConnection;
	long m_CEdit_Server_Set_Port;
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnEnChangeEdit2();
	virtual BOOL OnInitDialog();
	CButton m_CButton_Server_Set_Apply;
	afx_msg void OnBnClickedButtonApply();
};
