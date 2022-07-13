#pragma once
#include<windows.h>
#include<mmeapi.h>
#include<MMSYSTEM.H>
#include<MMReg.h>
#pragma  comment(lib,"Winmm.lib")
class CAudio
{
public:
	CAudio();
	virtual ~CAudio();
	BOOL     OnInitWaveIn();
	BOOL     OnInitWaveOut();
	LPBYTE   GetRecordBuffer(LPDWORD  BufferLength);
	static   DWORD  WINAPI  waveInCallBack(LPVOID lParam);
	BOOL     PlayRecordData(LPBYTE BufferData, DWORD BufferLength);
	GSM610WAVEFORMAT  m_GSMWavefmt;
	ULONG  m_BufferLength;
	HANDLE     m_ThreadCallBackHandle;

	LPWAVEHDR  m_InAudioHeader[2];   //����ͷ
	LPBYTE     m_InAudioData[2];     //��������  ��������������
	BOOL       m_IsWaveInUsed;
	BOOL	   m_IsWaveOutUsed;
	HWAVEIN    m_WaveInHandle;
	HWAVEOUT   m_WaveOutHandle;
	ULONG      m_WaveInIndex;
	ULONG	   m_WaveOutIndex;
	LPWAVEHDR  m_OutAudioHeader[2];   //����ͷ
	LPBYTE     m_OutAudioData[2];     //��������  ��������������
	HANDLE		m_EventHandle1;		//�����¼�
	HANDLE		m_EventHandle2;
};

