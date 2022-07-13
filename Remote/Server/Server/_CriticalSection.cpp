#include"stdafx.h"
#include "_CriticalSection.h"



_CCriticalSection::_CCriticalSection(CRITICAL_SECTION& CriticalSection)
{
	m_CriticalSection = &CriticalSection;
	Lock();
}


_CCriticalSection::~_CCriticalSection()
{
	Unlock();
}
void   _CCriticalSection::Lock()
{
	EnterCriticalSection(m_CriticalSection);
}
void   _CCriticalSection::Unlock()
{
	LeaveCriticalSection(m_CriticalSection);
}