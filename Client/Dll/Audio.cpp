#include "Audio.h"



CAudio::CAudio()
{
	m_IsWaveInUsed = FALSE;
	m_WaveInIndex = 0;
	m_IsWaveOutUsed = FALSE;
	m_WaveOutIndex = 0;

	//创建两个事件
	m_EventHandle1 = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_EventHandle2 = CreateEvent(NULL, TRUE, FALSE, NULL);
	//音频格式
	memset(&m_GSMWavefmt, 0, sizeof(GSM610WAVEFORMAT));
	m_GSMWavefmt.wfx.wFormatTag = WAVE_FORMAT_GSM610;
	m_GSMWavefmt.wfx.nChannels = 1;
	m_GSMWavefmt.wfx.nSamplesPerSec = 8000;
	m_GSMWavefmt.wfx.nAvgBytesPerSec = 1625;
	m_GSMWavefmt.wfx.nBlockAlign = 65;
	m_GSMWavefmt.wfx.wBitsPerSample = 0;
	m_GSMWavefmt.wfx.cbSize = 2;
	m_GSMWavefmt.wSamplesPerBlock = 320;

	m_BufferLength = 1000;
	int i = 0;
	for (i = 0; i < 2; i++)
	{
		m_InAudioHeader[i] = new WAVEHDR;
		m_InAudioData[i] = new BYTE[m_BufferLength];

		m_OutAudioHeader[i] = new WAVEHDR;
		m_OutAudioData[i] = new BYTE[m_BufferLength];
	}
}


CAudio::~CAudio()
{
	int  i = 0;
	if (m_IsWaveInUsed)
	{
		waveInStop(m_WaveInHandle);
		waveInReset(m_WaveInHandle);
		for (i = 0; i < 2; i++)
		{
			waveInUnprepareHeader(m_WaveInHandle, m_InAudioHeader[i], sizeof(WAVEHDR));
		}
		waveInClose(m_WaveInHandle);
		TerminateThread(m_ThreadCallBackHandle, -1);
	}
	for (i = 0; i < 2; i++)
	{
		delete[]  m_InAudioData[i];
		m_InAudioData[i] = NULL;
		delete[]  m_InAudioHeader[i];
		m_InAudioHeader[i] = NULL;
	}



	for (i = 0; i < 2; i++)
	{
		delete[]  m_OutAudioData[i];
		m_OutAudioData[i] = NULL;
		delete[]  m_OutAudioHeader[i];
		m_OutAudioHeader[i] = NULL;
	}
	if (m_IsWaveOutUsed)
	{
		waveOutReset(m_WaveOutHandle);
		for (int i = 0; i < 2; i++)
		{
			waveOutUnprepareHeader(m_WaveOutHandle, m_InAudioHeader[i], sizeof(WAVEHDR));
		}
		waveOutClose(m_WaveOutHandle);
	}
	CloseHandle(m_EventHandle1);
	CloseHandle(m_EventHandle2);
	CloseHandle(m_ThreadCallBackHandle);
}
LPBYTE   CAudio::GetRecordBuffer(LPDWORD  BufferLength)
{
	//录音机
	if (m_IsWaveInUsed == FALSE && OnInitWaveIn() == FALSE)
	{
		return NULL;
	}
	if (BufferLength == NULL)
	{
		return NULL;
	}
	SetEvent(m_EventHandle1);
	WaitForSingleObject(m_EventHandle2, INFINITE);
	ResetEvent(m_EventHandle1);
	*BufferLength = m_BufferLength;
	//返回真正数据
	return m_InAudioData[m_WaveInIndex];
}
BOOL     CAudio::OnInitWaveIn()
{
	MMRESULT  mmResult;
	DWORD     ThreadID = 0;
	m_ThreadCallBackHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)waveInCallBack, (LPVOID)this,
		CREATE_SUSPENDED, &ThreadID);

	//打开录音设备COM 1.指定声音规格  2.支持通过线程回调 换缓冲
	mmResult = waveInOpen(&m_WaveInHandle, (WORD)WAVE_MAPPER,
		&(m_GSMWavefmt.wfx), (LONG)ThreadID, (LONG)0, CALLBACK_THREAD);
	//m_WaveInHandle  录音机句柄
	if (mmResult != MMSYSERR_NOERROR)
	{
		return FALSE;
	}
	for (int i = 0; i < 2; i++)
	{
		m_InAudioHeader[i]->lpData = (LPSTR)m_InAudioData[i];
		m_InAudioHeader[i]->dwBufferLength = m_BufferLength;
		m_InAudioHeader[i]->dwFlags = 0;
		m_InAudioHeader[i]->dwLoops = 0;
		waveInPrepareHeader(m_WaveInHandle, m_InAudioHeader[i], sizeof(WAVEHDR));
	}
	waveInAddBuffer(m_WaveInHandle, m_InAudioHeader[m_WaveInIndex], sizeof(WAVEHDR));
	ResumeThread(m_ThreadCallBackHandle);
	waveInStart(m_WaveInHandle);   //录音
	m_IsWaveInUsed = TRUE;
	return TRUE;

}
DWORD  WINAPI  CAudio::waveInCallBack(LPVOID lParam)
{
	CAudio *This = (CAudio *)lParam;
	MSG Msg;
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		if (Msg.message == MM_WIM_DATA)
		{
			SetEvent(This->m_EventHandle2);
			WaitForSingleObject(This->m_EventHandle1, INFINITE);
			ResetEvent(This->m_EventHandle2);
			Sleep(1);
			This->m_WaveInIndex = 1 - This->m_WaveInIndex;
			//更新缓冲
			MMRESULT MMResult = waveInAddBuffer(This->m_WaveInHandle,
				This->m_InAudioHeader[This->m_WaveInIndex], sizeof(WAVEHDR));
			if (MMResult != MMSYSERR_NOERROR)
			{
				return  -1;
			}
		}
		if (Msg.message == MM_WIM_CLOSE)
		{
			break;
		}
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return 0;
}
BOOL CAudio::PlayRecordData(LPBYTE BufferData, DWORD BufferLength)
{
	//音频格式    播音设备
	if (!m_IsWaveOutUsed && !OnInitWaveOut())
	{
		return NULL;
	}
	for (int i = 0; i < BufferLength; i += m_BufferLength)
	{
		memcpy(m_OutAudioData[m_WaveOutIndex], BufferData, m_BufferLength);
		waveOutWrite(m_WaveOutHandle, m_OutAudioHeader[m_WaveOutIndex], sizeof(WAVEHDR));
		m_WaveOutIndex = 1 - m_WaveOutIndex;
	}
	return TRUE;
}
BOOL CAudio::OnInitWaveOut()
{
	if (!waveOutGetNumDevs())
	{
		return FALSE;
	}
	int i;
	for (i = 0; i < 2; i++)
	{
		memset(m_OutAudioData[i], 0, m_BufferLength);	//声音数据
	}

	MMRESULT MMResult;
	MMResult = waveOutOpen(&m_WaveOutHandle, (WORD)WAVE_MAPPER,
		&(m_GSMWavefmt.wfx), (LONG)0, (LONG)0, CALLBACK_NULL);

	if (MMResult != MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	for (int i = 0; i< 2; i++)
	{
		//关联
		m_OutAudioHeader[i]->lpData = (LPSTR)m_OutAudioData[i];
		m_OutAudioHeader[i]->dwBufferLength = m_BufferLength;
		m_OutAudioHeader[i]->dwFlags = 0;
		m_OutAudioHeader[i]->dwLoops = 0;
		waveOutPrepareHeader(m_WaveOutHandle, m_OutAudioHeader[i], sizeof(WAVEHDR));
	}

	m_IsWaveOutUsed = TRUE;
	return TRUE;
}