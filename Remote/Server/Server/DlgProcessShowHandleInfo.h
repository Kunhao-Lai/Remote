#pragma once
#include "afxcmn.h"
#include"Common.h"
#include"_IOCPServer.h"
// CDlgProcessShowHandleInfo �Ի���

class CDlgProcessShowHandleInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgProcessShowHandleInfo)

public:
	CDlgProcessShowHandleInfo(CWnd* pParent = NULL, _CIOCPServer*  IOCPServer = NULL, PCONTEXT_OBJECT  ContextObject = NULL);   // ��׼���캯��
	virtual ~CDlgProcessShowHandleInfo();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROCESS_SHOW_HANDLE_INFO };
#endif
private:
	_CIOCPServer*    m_IOCPServer;
	PCONTEXT_OBJECT  m_ContextObject;
	HICON    m_hIcon;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_CListCtrl_Dialog_Process_Handle_Info_Show;
	virtual BOOL OnInitDialog();
	void  ShowProcessHandleList(void);
};
