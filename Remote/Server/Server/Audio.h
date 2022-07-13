#pragma once
//#include<windows.h>
#include<MMSYSTEM.H>8
#include<MMReg.h>
#pragma  comment(lib,"Winmm.lib")
class CAudio
{
public:
	CAudio();
	~CAudio();
	BOOL     PlayRecordData(LPBYTE BufferData, DWORD BufferLength);
	BOOL     OnInitWaveOut();
private:
	GSM610WAVEFORMAT  m_GSMWavefmt;
	BOOL        m_IsWaveInUsed;
	BOOL        m_IsWaveOutUsed;
	ULONG       m_BufferLength;
	LPWAVEHDR   m_InAudioHeader[2];   //两个头
	LPBYTE      m_InAudioData[2];     //两个数据  保持声音的连续
	HANDLE		m_EventHandle1;		//两个事件
	HANDLE		m_EventHandle2;
	HWAVEIN    m_WaveInHandle;
	HWAVEOUT   m_WaveOutHandle;
	ULONG      m_WaveInIndex;
	ULONG	   m_WaveOutIndex;
	HANDLE     m_ThreadCallBackHandle;
	LPWAVEHDR  m_OutAudioHeader[2];   //两个头
	LPBYTE     m_OutAudioData[2];     //两个数据  保持声音的连续
};

