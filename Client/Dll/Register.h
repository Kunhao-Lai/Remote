#pragma once
#include<windows.h>
#include"Common.h"
enum  MYKEY {
	MHKEY_CLASSES_ROOT,
	MHKEY_CURRENT_USER,
	MHKEY_LOCAL_MACHINE,
	MHKEY_USERS,
	MHKEY_CURRENT_CONFIG
};
enum  KEYVALUE {
	MREG_SZ,
	MREG_DWORD,
	MREG_BINARY,
	MREG_EXPAND_SZ
};
struct  PACKET_HEADER {
	int  PathCount;      //名字个数
	DWORD  PathLength;     //名字大小
	DWORD  PathName;  //值大小
};
class CRegister
{
public:
	CRegister(char  IsToken);
	~CRegister();
	char*  FindPath();
	void   SetPath(char* FullPath);
	char*  FindKey();
private:
	char  m_KeyPath[MAX_PATH];
	HKEY  m_KeyHandle;
};

