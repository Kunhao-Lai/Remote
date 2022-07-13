#pragma once
#include"_IOCPServer.h"
#include"DlgProcessShowThreadInfo.h"
#include"DlgProcessShowHandleInfo.h"
#include"Common.h"
#include "afxcmn.h"

// CDlgProcessManager 对话框

class CDlgProcessManager : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgProcessManager)

public:
	CDlgProcessManager(CWnd* pParent = NULL,_CIOCPServer*  IOCPServer = NULL,PCONTEXT_OBJECT  ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgProcessManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROCESS_MANAGER };
#endif
private:
	_CIOCPServer*    m_IOCPServer;
	PCONTEXT_OBJECT  m_ContextObject;
	HICON    m_hIcon;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_CListCtrl_Dialog_Process_Manager_Show;
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void  ShowProcessList(void);
	void  WindowHandleIO(void);
	afx_msg void OnNMCustomdrawListDialogProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListDialogProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuDialogProcessManagerListShowRefresh();
	afx_msg void OnMenuDialogProcessManagerTerminateProcess();
	afx_msg void OnMenuDialogProcessManagerShowModuleInfo();
	afx_msg void OnMenuDialogProcessManagerShowHandleInfo();
};
