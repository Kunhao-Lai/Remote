#pragma once


// CDlgCreateClient �Ի���

class CDlgCreateClient : public CDialog
{
	DECLARE_DYNAMIC(CDlgCreateClient)

public:
	CDlgCreateClient(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgCreateClient();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CREATE_CLIENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CString m_CEdit_Dialog_Create_Client_IP;                //����IP
	CString m_CEdit_Dialog_Create_Client_Port;              //���Ӷ˿�
	afx_msg void OnBnClickedOk();
	int  SeMemoryFind(const char* BufferData, const char* KeyValue, int BufferLength, int KeyLength);
};
