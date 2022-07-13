#pragma once
#include<iostream>
#include<windows.h>
using namespace std;

class _CCriticalSection
{
public:
	_CCriticalSection(CRITICAL_SECTION& CriticalSection);
	~_CCriticalSection();
	void   Lock();
	void   Unlock();
private:
	CRITICAL_SECTION*    m_CriticalSection;
};

