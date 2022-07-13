#pragma once
#include "afxcmn.h"
#include"Common.h"
#include"_IOCPServer.h"
// CDlgProcessShowHandleInfo 对话框

class CDlgProcessShowHandleInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgProcessShowHandleInfo)

public:
	CDlgProcessShowHandleInfo(CWnd* pParent = NULL, _CIOCPServer*  IOCPServer = NULL, PCONTEXT_OBJECT  ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgProcessShowHandleInfo();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROCESS_SHOW_HANDLE_INFO };
#endif
private:
	_CIOCPServer*    m_IOCPServer;
	PCONTEXT_OBJECT  m_ContextObject;
	HICON    m_hIcon;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_CListCtrl_Dialog_Process_Handle_Info_Show;
	virtual BOOL OnInitDialog();
	void  ShowProcessHandleList(void);
};
