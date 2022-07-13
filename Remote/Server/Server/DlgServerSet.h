#pragma once
#include "ConfigFile.h"
#include "afxwin.h"

// CDlgServerSet 对话框

class CDlgServerSet : public CDialog
{
	DECLARE_DYNAMIC(CDlgServerSet)

public:
	CDlgServerSet(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgServerSet();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SERVER_SET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
