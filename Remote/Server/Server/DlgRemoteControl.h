#pragma once
#include"Common.h"
#include"_IOCPServer.h"



extern "C" VOID CopyScreenData(PVOID  PreScreenData, PVOID CurrentBufferData, ULONG CurrentBufferLength);
// CDlgRemoteControl 对话框

class CDlgRemoteControl : public CDialog
{
	DECLARE_DYNAMIC(CDlgRemoteControl)

public:
	CDlgRemoteControl(CWnd* pParent = NULL, _CIOCPServer*  IOCPServer = NULL, PCONTEXT_OBJECT  ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgRemoteControl();

// 对话框数据
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
	POINT    m_ClientCursorPosition;                     //存储鼠标位置
	ULONG    m_HScrollPosition;
	ULONG    m_VScrollPosition;
	BOOL     m_IsTraceCursor;             //是否跟踪鼠标
	BOOL     m_IsControl;                 //是否控制屏幕
	HACCEL   m_hAccel_TraceCursor;           //快捷键（跟踪鼠标）
	HACCEL   m_hAccel_ControlScreen;           //快捷键（控制屏幕）
	HACCEL   m_hAccel_SaveSnapshot;           //快捷键（保存快照）
	HACCEL   m_hAccel_BlockInput;           //快捷键（锁定客户端）
	HACCEL   m_hAccel_GetClipBoard;           //快捷键（获取剪切板）
	HACCEL   m_hAccel_SetClipBoard;           //快捷键（设置剪切板）
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
