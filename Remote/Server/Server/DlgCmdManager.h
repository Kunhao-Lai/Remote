#pragma once
#include"Common.h"
#include"_IOCPServer.h"
#include "afxwin.h"

// CDlgCmdManager �Ի���

class CDlgCmdManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgCmdManager)

public:
	CDlgCmdManager(CWnd* pParent = NULL, _CIOCPServer*  IOCPServer = NULL, PCONTEXT_OBJECT  ContextObject = NULL);   // ��׼���캯��
	virtual ~CDlgCmdManager();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CMD_MANAGER };
#endif
private:
	_CIOCPServer*    m_IOCPServer;
	PCONTEXT_OBJECT  m_ContextObject;
	HICON    m_hIcon;
	ULONG    m_ShowDataLength;
	ULONG    m_911;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void  WindowHandleIO(void);
	CEdit m_CEdit_Dialog_Cmd_Manager_Show;
	void  ShowCmdData();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
