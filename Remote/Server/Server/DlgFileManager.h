#pragma once
#include"_IOCPServer.h"
#include"Common.h"
#include "TrueColorToolBar.h"
#include "afxcmn.h"
#include "afxwin.h"
// CDlgFileManager 对话框

typedef struct
{
	DWORD	FileSizeHigh;
	DWORD	FileSizeLow;
}FILE_SIZE;
#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))
typedef CList<CString, CString&> List;
class CDlgFileManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgFileManager)

public:
	CDlgFileManager(CWnd* pParent = NULL, _CIOCPServer*  IOCPServer = NULL, PCONTEXT_OBJECT  ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgFileManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_MANAGER };
#endif
private:
	_CIOCPServer*    m_IOCPServer;
	PCONTEXT_OBJECT  m_ContextObject;
	HICON    m_hIcon;
	BYTE     m_ClientData[0x1000];
	BYTE     m_ServerData[0x1000];
	
	CString  m_ServerFileFullPath;
	CString  m_ClientFileFullPath;
	//界面相关
	CStatusBar  m_StatusBar;
	CProgressCtrl*   m_ProgressCtrl;	
	CImageList*  m_CImageList_Small;
	CImageList*  m_CImageList_Large;
	CTrueColorToolBar   m_ToolBar_File_Manager_Server_File;
	CTrueColorToolBar   m_ToolBar_File_Manager_Client_File;
	CListCtrl m_CListCtrl_Dialog_File_Manager_Server_File;
	CListCtrl m_CListCtrl_Dialog_File_Manager_Client_File;
	CStatic m_CStatic_Dialog_File_Manager_Server_Position;
	CStatic m_CStatic_Dialog_File_Manager_Client_Position;
	CComboBox m_CCombox_Dialog_File_Manager_Server_File;
	CComboBox m_CCombox_Dialog_File_Manager_Client_File;
	//文件拷贝
	HCURSOR   m_CursorHwnd;
	BOOL      m_IsDragging;         //拖拽文件
	BOOL      m_IsSTop;            //拷贝文件的过程中可以停止
	CListCtrl*   m_CListCtrl_Drag;
	CListCtrl*   m_CListCtrl_Drog;
	List      m_ServerDataToClientJob;
	List      m_ClientDataToServerJob;
	CString   m_SourceFileFullPath;
	CString   m_DestinationFileFullPath;
	__int64   m_OperatingFileLength;      // 当前操作文件总大小
	ULONG     m_TransferMode;             //传送方式
	__int64   m_Counter;                  //计数器，控制进度条
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	void  WindowHandleIO(void);
	void  FixedServerVolumeData(); 
	void  FixedClientVolumeData();
	void  FixedServerFileData(CString  Directory = ""); 
	void  FixedClientFileData(BYTE*  BufferData, ULONG  BufferLength);
	void  GetClientFileData(CString  Directory = "");
	int   GetIconIndex(LPCTSTR  VolumeName, DWORD  FileAttributes);
	CString  GetParentDirectory(CString	  FileFullPath);
	
	
	afx_msg void OnNMDblclkListDialogFileManagerServerFile(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListDialogFileManagerClientFile(NMHDR *pNMHDR, LRESULT *pResult);
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnFileManagerServerFilePrevious();
	afx_msg void OnFileManagerServerFileDelete();
	afx_msg void OnFileManagerServerFileNewFolder();
	afx_msg void OnFileManagerServerFileStop();
	afx_msg void OnFileManagerServerFileViewSmall();
	afx_msg void OnFileManagerServerFileViewList();
	afx_msg void OnFileManagerServerFileViewDetail();
	afx_msg void OnFileManagerClientFilePrevious();
	afx_msg void OnFileManagerClientFileDelete();
	afx_msg void OnFileManagerClientFileNewFolder();
	afx_msg void OnFileManagerClientFileStop();
	afx_msg void OnFileManagerClientFileViewSmall();
	afx_msg void OnFileManagerClientFileViewList();
	afx_msg void OnFileManagerClientFileViewDetail();
	void EnableControl(BOOL IsEnable);
	BOOL DeleteDirectory(LPCTSTR DirectoryFullPath);
	BOOL MakeSureDirectoryPathExists(char* DirectoryFullPath);
	afx_msg void OnLvnBegindragListDialogFileManagerServerFile(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	VOID   DropItemOnList();
	VOID   OnCopyFileServerToClient();
	BOOL   FixedServerToClientDirectory(LPCTSTR DircetoryFullPath);
	BOOL   SendServerDataToClient();
	void   SendTransferMode();
	VOID   EndCopyServerFileToClient();
	void   ShowProgress();        //显示进度条
	VOID   SendServerFileDataToClient();
	VOID   OnCopyFileClientToServer();
	BOOL   SendClientDataToServer();
};
