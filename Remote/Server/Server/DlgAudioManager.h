#pragma once
#include"Common.h"
#include"_IOCPServer.h"
#include"Audio.h"

// CDlgAudioManager 对话框

class CDlgAudioManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgAudioManager)

public:
	CDlgAudioManager(CWnd* pParent = NULL, _CIOCPServer*  IOCPServer = NULL, PCONTEXT_OBJECT  ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgAudioManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_AUDIO_MANAGER };
#endif
private:
	_CIOCPServer*    m_IOCPServer;
	PCONTEXT_OBJECT  m_ContextObject;
	HICON    m_hIcon;
	CAudio   m_Audio;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void  WindowHandleIO(void);
};
