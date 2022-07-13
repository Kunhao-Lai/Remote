#include"stdafx.h"
#include "_Array.h"

#define  U_PAGE_ALIGNMENT   3
#define  F_PAGE_ALIGNMENT   3.0

_CArray::_CArray()
{
	m_MaximumLength = 0;
	m_BufferData = m_CheckPoint = NULL;
	InitializeCriticalSection(&m_CriticalSection);
}


_CArray::~_CArray()
{
	if (m_BufferData)
	{
		VirtualFree(m_BufferData, 0, MEM_RELEASE);
		m_BufferData = NULL;
	}
	DeleteCriticalSection(&m_CriticalSection);
	m_BufferData = m_CheckPoint = NULL;
	m_MaximumLength = 0;
}
BOOL   _CArray::WriteArray(PUINT8  BufferData, ULONG_PTR  BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);
	if (ReallocateArray(BufferLength + GetArrayLength()) == (ULONG_PTR)-1)
	{
		LeaveCriticalSection(&m_CriticalSection);
		return FALSE;
	}
	CopyMemory(m_CheckPoint, BufferData, BufferLength);
	m_CheckPoint += BufferLength;
	LeaveCriticalSection(&m_CriticalSection);
	return TRUE;
}
ULONG_PTR   _CArray::ReallocateArray(ULONG_PTR  BufferLength)
{
	if (BufferLength < GetArrayMaximumLength())
	{
		return 0;
	}
	//向上取边界
	ULONG_PTR  v7 = (ULONG_PTR)ceil(BufferLength / F_PAGE_ALIGNMENT)*U_PAGE_ALIGNMENT;
	PUINT8     v5 = (PUINT8)VirtualAlloc(NULL, v7, MEM_COMMIT, PAGE_READWRITE);
	if (v5 == NULL)
	{
		return -1;
	}
	//将原来的数据拷贝到新申请的内存中
	ULONG_PTR v3 = GetArrayLength();                    //原来的有效数据长度
	CopyMemory(v5, m_BufferData, v3);

	if (m_BufferData)
	{
		VirtualFree(m_BufferData, 0, MEM_RELEASE);
	}
	m_BufferData = v5;
	m_CheckPoint = m_BufferData + v3;
	m_MaximumLength = v7;
	return m_MaximumLength;
}
ULONG_PTR   _CArray::GetArrayMaximumLength()
{
	return m_MaximumLength;
}
ULONG_PTR   _CArray::GetArrayLength()
{
	if (m_BufferData == NULL)
	{
		return 0;
	}
	return (ULONG_PTR)m_CheckPoint - (ULONG_PTR)m_BufferData;
}
PUINT8      _CArray::GetArray(ULONG_PTR  Position )
{
	if (m_BufferData == NULL)
	{
		return NULL;
	}
	if (Position > GetArrayLength())
	{
		return NULL;
	}
	return  m_BufferData + Position;
}
void        _CArray::ClearArray()
{
	EnterCriticalSection(&m_CriticalSection);
	m_CheckPoint = m_BufferData;
	DeallocateArray(1024);
	LeaveCriticalSection(&m_CriticalSection);
}
ULONG_PTR   _CArray::DeallocateArray(ULONG_PTR  BufferLength)
{
	if (BufferLength < GetArrayLength())
	{
		return 0;
	}
	ULONG_PTR  v7 = (ULONG_PTR)ceil(BufferLength / F_PAGE_ALIGNMENT)*U_PAGE_ALIGNMENT;
	if (m_MaximumLength <= v7)
	{
		return 0;
	}
	PUINT8     v5 = (PUINT8)VirtualAlloc(NULL, v7, MEM_COMMIT, PAGE_READWRITE);
	ULONG_PTR v3 = GetArrayLength();                    //原来的有效数据长度
	CopyMemory(v5, m_BufferData, v3);
	VirtualFree(m_BufferData, 0, MEM_RELEASE);
	m_BufferData = v5;
	m_CheckPoint = m_BufferData + v3;
	m_MaximumLength = v7;
	return m_MaximumLength;
}
ULONG_PTR   _CArray::ReadArray(PUINT8  BufferData, ULONG_PTR  BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);
	if (BufferLength > m_MaximumLength)
	{
		LeaveCriticalSection(&m_CriticalSection);
		return 0;
	}
	if (BufferLength > GetArrayLength())
	{
		BufferLength = GetArrayLength();
	}
	if (BufferLength)
	{
		CopyMemory(BufferData, m_BufferData, BufferLength);
		MoveMemory(m_BufferData, m_BufferData + BufferLength, GetArrayMaximumLength() - BufferLength);
		m_CheckPoint -= BufferLength;
	}
	DeallocateArray(GetArrayLength());
	LeaveCriticalSection(&m_CriticalSection);
	return BufferLength;
}
ULONG_PTR   _CArray::RemoveComletedArray(ULONG_PTR  BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);
	if (BufferLength > GetArrayMaximumLength())
	{
		return 0;
	}
	if (BufferLength > GetArrayLength())
	{
		BufferLength = GetArrayLength();
	}
	if (BufferLength)
	{
		MoveMemory(m_BufferData, m_BufferData + BufferLength, GetArrayMaximumLength() - BufferLength);
		m_CheckPoint -= BufferLength;
	}
	DeallocateArray(GetArrayLength());
	LeaveCriticalSection(&m_CriticalSection);
	return BufferLength;
}