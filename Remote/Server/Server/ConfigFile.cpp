#include"stdafx.h"
#include "ConfigFile.h"



CConfigFile::CConfigFile()
{
	InitConfigFile();
}


CConfigFile::~CConfigFile()
{
}
BOOL  CConfigFile::InitConfigFile()
{
	CHAR  FileFullPath[MAX_PATH] = { 0 };
	//获得当前可执行文件的绝对路径
	::GetModuleFileNameA(NULL, FileFullPath, MAX_PATH);
	CHAR* v1 = NULL;
	v1 = strstr(FileFullPath, ".");
	if (v1 != NULL)
	{
		*v1 = '\0';
		strcat(FileFullPath, ".ini");
	}
	HANDLE  FileHandle = CreateFileA(FileFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	m_FileFullPath = FileFullPath;
	ULONG  HighLength = 0;
	ULONG  LowLength = GetFileSize(FileHandle, &HighLength);
	if (LowLength > 0 || HighLength > 0)
	{
		CloseHandle(FileHandle);
		return  FALSE;
	}
	CloseHandle(FileHandle);
	WritePrivateProfileStringA("Settings", "ListenPort", "2356", m_FileFullPath.c_str());
	WritePrivateProfileStringA("Settings", "MaxConnection", "20", m_FileFullPath.c_str());
	
	return  TRUE;
}
int   CConfigFile::GetInt(string  MainKey, string  SubKey)
{
	return ::GetPrivateProfileIntA(MainKey.c_str(), SubKey.c_str(), 0, m_FileFullPath.c_str());
}
BOOL  CConfigFile::SetInt(string  MainKey, string SubKey, int BufferData)
{
	string v1;
	sprintf((char*)v1.c_str(), "%d", BufferData);
	return ::WritePrivateProfileStringA(MainKey.c_str(), SubKey.c_str(), v1.c_str(), m_FileFullPath.c_str());
}