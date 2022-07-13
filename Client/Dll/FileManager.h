#pragma once
#include "Manager.h"

typedef struct
{
	DWORD	FileSizeHigh;
	DWORD	FileSizeLow;
}FILE_SIZE;

class CFileManager :
	public CManager
{
private:
	char    m_FileFullPath[MAX_PATH];
	__int64 m_OperatingFileLength;                //正在操作的文件大小
	ULONG   m_TransferMode;
public:
	CFileManager(_CIOCPClient* IOCPClient);
	~CFileManager();
	virtual  void  HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength);
	ULONG  SendClientVolumeList();
	int  SendClientFileData(PBYTE  FileFullPath);
	void  DeleteClientFile(PBYTE  Directory);
	BOOL DeleteDirectory(LPCTSTR DirectoryFullPath);
	VOID CreateReceivedFile(LPBYTE BufferData);
	BOOL MakeSureDirectoryPathExists(char* DirectoryFullPath);
	VOID SetTransferMode(LPBYTE BufferData);
	VOID GetFileData();
	VOID WriteReceivedFileData(LPBYTE BufferData, ULONG BufferLength);
};

