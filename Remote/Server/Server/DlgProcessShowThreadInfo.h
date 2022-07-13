#pragma once
#include "afxcmn.h"
#include"_IOCPServer.h"
#include"Common.h"

// CDlgProcessShowThreadInfo �Ի���
typedef LONG KPRIORITY, *PKPRIORITY;
class CDlgProcessShowThreadInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgProcessShowThreadInfo)

public:
	CDlgProcessShowThreadInfo(CWnd* pParent = NULL, _CIOCPServer*  IOCPServer = NULL, PCONTEXT_OBJECT  ContextObject = NULL);   // ��׼���캯��
	virtual ~CDlgProcessShowThreadInfo();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROCESS_SHOW_THREAD_INFO };
#endif
private:
	_CIOCPServer*    m_IOCPServer;
	PCONTEXT_OBJECT  m_ContextObject;
	HICON    m_hIcon;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_CListCtrl_Dialog_Process_Thread_Info_Show;
	void  ShowProcessThreadList(void);
	afx_msg void OnClose();
};
