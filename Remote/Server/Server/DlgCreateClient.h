#pragma once


// CDlgCreateClient 对话框

class CDlgCreateClient : public CDialog
{
	DECLARE_DYNAMIC(CDlgCreateClient)

public:
	CDlgCreateClient(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgCreateClient();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CREATE_CLIENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_CEdit_Dialog_Create_Client_IP;                //连接IP
	CString m_CEdit_Dialog_Create_Client_Port;              //连接端口
	afx_msg void OnBnClickedOk();
	int  SeMemoryFind(const char* BufferData, const char* KeyValue, int BufferLength, int KeyLength);
};
