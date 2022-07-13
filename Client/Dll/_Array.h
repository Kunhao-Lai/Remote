#pragma once
#include<iostream>
#include<windows.h>
#include<math.h>
class _CArray
{
public:
	_CArray();
	~_CArray();
	BOOL        WriteArray(PUINT8  BufferData, ULONG_PTR  BufferLength);
	ULONG_PTR   ReallocateArray(ULONG_PTR  BufferLength);
	ULONG_PTR   GetArrayMaximumLength();                             //������ݳ���
	ULONG_PTR   GetArrayLength();                                    //ԭ������Ч���ݳ���
	PUINT8      GetArray(ULONG_PTR  Position = 0);
	void        ClearArray();
	ULONG_PTR   DeallocateArray(ULONG_PTR  BufferLength);
	ULONG_PTR   ReadArray(PUINT8  BufferData, ULONG_PTR  BufferLength);
	ULONG_PTR   RemoveComletedArray(ULONG_PTR  BufferLength);
private:
	PUINT8      m_BufferData;
	PUINT8      m_CheckPoint;
	ULONG_PTR   m_MaximumLength;
	CRITICAL_SECTION  m_CriticalSection;
};

