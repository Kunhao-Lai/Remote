#pragma once
#include"Common.h"
#include"_IOCPServer.h"
#include "afxcmn.h"

// CDlgRegisterManager 对话框

class CDlgRegisterManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgRegisterManager)

public:
	CDlgRegisterManager(CWnd* pParent = NULL, _CIOCPServer*  IOCPServer = NULL, PCONTEXT_OBJECT  ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgRegisterManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REGISTER_MANAGER };
#endif
private:
	_CIOCPServer*    m_IOCPServer;
	PCONTEXT_OBJECT  m_ContextObject;
	HICON    m_hIcon;
	HTREEITEM   m_RootTreeItem;
	HTREEITEM   m_CurrentUserTreeItem;
	HTREEITEM   m_LocalMachineTreeItem;
	HTREEITEM   m_UsersTreeItem;
	HTREEITEM   m_CurrentConfigTreeItem;
	HTREEITEM   m_ClassRootTreeItem;
	HTREEITEM   m_SelectedTreeItem;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	void  WindowHandleIO(void);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	CTreeCtrl m_CTreeCtrl_Dialog_Register_Manager_Show;
	CListCtrl m_CListCtrl_Dialog_Register_Manager_Show;
	CImageList  m_ImageListTree;
	CImageList  m_CImageList_List;
	BOOL    m_IsEnable;
	afx_msg void OnTvnSelchangedTreeDialogRegisterManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	char    GetFatherPath(CString&  SelectedTreeItem);
	CString   GetFullPath(HTREEITEM  SelectedTreeItem);
	void   AddPath(char*  BufferData);
	void   AddKey(char*  BufferData);
};
