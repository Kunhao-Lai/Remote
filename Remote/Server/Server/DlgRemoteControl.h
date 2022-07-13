#pragma once
#include"Common.h"
#include"_IOCPServer.h"



extern "C" VOID CopyScreenData(PVOID  PreScreenData, PVOID CurrentBufferData, ULONG CurrentBufferLength);
// CDlgRemoteControl �Ի���

class CDlgRemoteControl : public CDialog
{
	DECLARE_DYNAMIC(CDlgRemoteControl)

public:
	CDlgRemoteControl(CWnd* pParent = NULL, _CIOCPServer*  IOCPServer = NULL, PCONTEXT_OBJECT  ContextObject = NULL);   // ��׼���캯��
	virtual ~CDlgRemoteControl();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REMOTE_CONTROL };
#endif
private:
	_CIOCPServer*    m_IOCPServer;
	PCONTEXT_OBJECT  m_ContextObject;
	HICON    m_hIcon;
	LPBITMAPINFO  m_BitmapInfo;
	HDC      m_DesktopDCHandle;
	HDC      m_DesktopMemoryDCHandle;
	HBITMAP  m_BitmapHandle;
	PVOID    m_BitmapData;
	POINT    m_ClientCursorPosition;                     //�洢���λ��
	ULONG    m_HScrollPosition;
	ULONG    m_VScrollPosition;
	BOOL     m_IsTraceCursor;             //�Ƿ�������
	BOOL     m_IsControl;                 //�Ƿ������Ļ
	HACCEL   m_hAccel_TraceCursor;           //��ݼ���������꣩
	HACCEL   m_hAccel_ControlScreen;           //��ݼ���������Ļ��
	HACCEL   m_hAccel_SaveSnapshot;           //��ݼ���������գ�
	HACCEL   m_hAccel_BlockInput;           //��ݼ��������ͻ��ˣ�
	HACCEL   m_hAccel_GetClipBoard;           //��ݼ�����ȡ���а壩
	HACCEL   m_hAccel_SetClipBoard;           //��ݼ������ü��а壩
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void  WindowHandleIO(void);
	void  DrawFirstScreen();
	void  DrawNextScreen();
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	VOID   SendCommand(MSG*  Msg);
	BOOL   SaveSnapshot();
	VOID   UpdateClipboard(char* BufferData, ULONG  BufferLength);
	VOID   SendClipboardData();
	afx_msg void OnAcceleratorTraceCursor();
	afx_msg void OnAcceleratorControlScreen();
	afx_msg void OnAcceleratorSaveDib();
	afx_msg void OnAcceleratorBlockInput();
	afx_msg void OnAcceleratorGetClipboard();
	afx_msg void OnAcceleratorSetClipboard();
};
