#pragma once
#include"_IOCPServer.h"
#include "afxwin.h"


// CDlgRemoteMessage 对话框

class CDlgRemoteMessage : public CDialog
{
	DECLARE_DYNAMIC(CDlgRemoteMessage)

public:
	CDlgRemoteMessage(CWnd* pParent = NULL,_CIOCPServer* IOCPServer = NULL,
		PCONTEXT_OBJECT  ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgRemoteMessage();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALIG_REMOTE_MESSAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	_CIOCPServer*    m_IOCPServer;
	PCONTEXT_OBJECT  m_ContextObject;
	CEdit    m_CEdit_Dialog_Remote_Message;
	HICON    m_hIcon;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void   OnSending();
};
