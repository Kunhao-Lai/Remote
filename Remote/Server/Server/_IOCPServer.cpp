#include "stdafx.h"
#include "_IOCPServer.h"


_CIOCPServer::_CIOCPServer()
{
	//��ʼ���׽������
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		return;
	}
	memcpy(m_PacketHeaderFlag, "Shine", PACKET_FLAG_LENGTH);
	m_ListenSocket = INVALID_SOCKET;
	m_ListenThreadHandle = NULL;
	m_ListenEventHandle = WSA_INVALID_EVENT;
	InitializeCriticalSection(&m_CriticalSection);
	m_CompletionPortHandle = INVALID_HANDLE_VALUE;
	m_KillEventHandle = NULL;

	m_ThreadPoolMax = 0;
	m_ThreadPoolMin = 0;
	m_WorkThreadCount = 0;
	m_CurrentThreadCount = 0;
	m_BusyThreadCount = 0;
	m_IsWorking = FALSE;

}
_CIOCPServer::~_CIOCPServer()
{
	Sleep(10);
	m_ThreadPoolMax = 0;
	m_ThreadPoolMin = 0;
	m_WorkThreadCount = 0;
	m_BusyThreadCount = 0;
	m_CurrentThreadCount = 0;
	m_IsWorking = FALSE;
	//�����¼�ʹ�������˳������̵߳�ѭ��
	SetEvent(m_KillEventHandle);
	//�ȴ������߳��˳�
	//SetEvent(m_ListenEventHandle);
	WaitForSingleObject(m_ListenThreadHandle, INFINITE);

	//������Դ
	if (m_ListenSocket != INVALID_SOCKET)
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}
	if (m_ListenEventHandle != WSA_INVALID_EVENT)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
	}
	if (m_CompletionPortHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_CompletionPortHandle);
		m_CompletionPortHandle = INVALID_HANDLE_VALUE;
	}

	//���ٹؼ������
	DeleteCriticalSection(&m_CriticalSection);
	WSACleanup();

}
BOOL   _CIOCPServer::ServerRun(USHORT  ListenPort, LPFNWINDOWNOTIFYPROCEDURE  WindowNotifyProcedure)
{		
	m_WindowNotifyProcedure = WindowNotifyProcedure;
	//�����˳������̵߳��¼�
	m_KillEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_KillEventHandle == NULL)
	{
		return  FALSE;
	}
	//����һ�������׽���
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_ListenSocket == INVALID_SOCKET)
	{
		return  FALSE;
	}
	//����һ�������¼�
	m_ListenEventHandle = WSACreateEvent();
	DWORD LastError = GetLastError();
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return  FALSE;
	}
	//�������׽������¼����й���
	BOOL  IsOk = WSAEventSelect(m_ListenSocket, m_ListenEventHandle, FD_ACCEPT | FD_CLOSE);
	if (IsOk == SOCKET_ERROR)
	{
		goto  Error;
	}
	//��ʼ��Server������
	SOCKADDR_IN  ServerAddress;
	ServerAddress.sin_port = htons(ListenPort);
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	//���׽���
	IsOk = bind(m_ListenSocket, (sockaddr*)&ServerAddress,sizeof(ServerAddress));
	if (IsOk == SOCKET_ERROR)
	{
		goto  Error;
	}
	//����
	IsOk = listen(m_ListenSocket, SOMAXCONN);
	if (IsOk == SOCKET_ERROR)
	{
		goto  Error;
	}
	//���������߳�
	m_ListenThreadHandle =
		(HANDLE)CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)ListenThreadProcedure,
			(void*)this,
			0,
			NULL);
	if (m_ListenThreadHandle == INVALID_HANDLE_VALUE)
	{
		goto  Error;
	}
	
	//��ʼ��IOCP
	InitializeIOCP();
	return  TRUE;
Error:
	WSACloseEvent(m_ListenEventHandle);
	m_ListenEventHandle = WSA_INVALID_EVENT;
	closesocket(m_ListenSocket);
	m_ListenSocket = INVALID_SOCKET;
	return  FALSE;
}
DWORD WINAPI  _CIOCPServer::ListenThreadProcedure(LPVOID  ParameterData)
{
	_CIOCPServer* This = (_CIOCPServer*)ParameterData;
	int EventIndex = 0;
	WSANETWORKEVENTS  NetWorkEvents;
	while (1)
	{
		EventIndex = WaitForSingleObject(This->m_KillEventHandle, 100);
		EventIndex = EventIndex - WAIT_OBJECT_0;
		if (EventIndex == 0)
		{
			//����������������m_KillEventHandle�¼�
			break;
		}
		DWORD v1;
		//�ȴ������¼����ţ������׽������ţ�
		v1 = WSAWaitForMultipleEvents(1,
			&This->m_ListenEventHandle,
			FALSE,
			100,
			FALSE);
		DWORD LastError = GetLastError();
		if (v1 == WSA_WAIT_TIMEOUT)
		{
			//���¼�û������
			continue;
		}
		//����¼��������Ǿͽ����¼�ת����һ�������¼������ж�
		v1 = WSAEnumNetworkEvents(This->m_ListenSocket,
			This->m_ListenEventHandle,
			&NetWorkEvents);
		if (v1 == SOCKET_ERROR)
		{
			break;
		}
		if (NetWorkEvents.lNetworkEvents&FD_ACCEPT)  //�����׽�������
		{
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				//���ͻ�������������
				This->OnAccept();
			}
			else
			{
				break;
			}
		}
		else
		{
			//��ɾ��һ���û�ʱ�������ִ�е�����
		//	break;
		}
	}
	return 0;
}
void    _CIOCPServer::OnAccept()
{
	int  Result = 0;
	//���������û�IP��ַ
	SOCKET  ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN  ClientAddress = { 0 };
	int    ClientAddressLength = sizeof(SOCKADDR_IN);

	ClientSocket = accept(m_ListenSocket,          //ͬ������������Ϣ
		(sockaddr*)&ClientAddress,
		&ClientAddressLength);

	if (ClientSocket == SOCKET_ERROR)
	{
		/*Result = WSAGetLastError();
		if (Result != WSAEWOULDBLOCK)
		{
			return;
		}*/
		return;
	}
	PCONTEXT_OBJECT  ContextObject = AllocateContextObiect();
	if (ContextObject == NULL)
	{
		closesocket(ClientSocket);
		m_ListenSocket = INVALID_SOCKET;
		return;
	}

	//��Ա��ֵ
	ContextObject->ClientSocket = ClientSocket;
	//�����ڴ棨wsaRecv���ڽ��տͻ����������ݣ�
	ContextObject->ReceiveWsaInBuffer.buf = (char*)ContextObject->m_BufferData;
	ContextObject->ReceiveWsaInBuffer.len = sizeof(ContextObject->m_BufferData);

	//�����ɵ�ͨѶ�׽�������ɶ˿ھ�������
	HANDLE Handle = CreateIoCompletionPort((HANDLE)ClientSocket,
		m_CompletionPortHandle, (ULONG_PTR)ContextObject, 0);
	if (Handle != m_CompletionPortHandle)
	{
		delete ContextObject;
		ContextObject = NULL;
		if (ClientSocket != INVALID_SOCKET)
		{
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
		}
		return;
	}

	//�������ӣ����˫�������Ƿ����������Сʱ���ڴ��׽ӿڵ���һ����û�����ݽ���
	//TCP�Զ����Է���һ�����ִ��

	//�����׽��ֵ�ѡ�Set KeepAlive ����������� SO_KEEPALIVE
	m_KeepAliveTime = 3;
	BOOL  IsOk = TRUE;

	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsOk, sizeof(IsOk)) != 0)
	{

	}
	
	//���ó�ʱ��ϸ��Ϣ
	tcp_keepalive  KeepAlive;
	KeepAlive.onoff = 1;                                 //���ñ���
	KeepAlive.keepalivetime = m_KeepAliveTime;           //����������û�����ݣ��ͷ���̽���
	KeepAlive.keepaliveinterval = 1000 * 10;             //���Լ��Ϊ10��

	WSAIoctl(ContextObject->ClientSocket,
		SIO_KEEPALIVE_VALS,
		&KeepAlive,
		sizeof(KeepAlive),
		NULL,
		0,
		(unsigned  long*)&IsOk,
		0,
		NULL);

	_CCriticalSection  CriticalSection(m_CriticalSection);
	m_ConnectionContextObjectList.AddTail(ContextObject);

	OVERLAPPEDEX*  OverlappedEx = new  OVERLAPPEDEX(IO_INITIALIZE);
	IsOk = FALSE;
	//����ɶ˿�Ͷ��һ������
	//�����̻߳�ȴ���ɶ˿ڵ����״̬
	IsOk = PostQueuedCompletionStatus(m_CompletionPortHandle,
		0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);
	//���Ͷ��ʧ��
	if ((!IsOk&&GetLastError() != ERROR_IO_PENDING))
	{
		RemoveContextObject(ContextObject);
		return;
	}
	//�������û�����������ߵ�����
	//����������û�Ͷ��PostRecv����
	PostRecv(ContextObject);
}
void  _CIOCPServer::PostRecv(CONTEXT_OBJECT*  ContextObject)
{
	//�����Ǹ����ߵ��û�Ͷ��һ���������ݵ�����
	//����û��ĵ�һ�����ݰ����Ｔ���ض˵ĵ�½���󵽴����ǵĹ����߳̾ͻ���Ӧ������ProcessIOMessage����
	OVERLAPPEDEX*  OverlappedEx = new  OVERLAPPEDEX(IO_RECEIVE);

	DWORD  ReternLength;
	ULONG  Flags = MSG_PARTIAL;
	int    IsOk = WSARecv(ContextObject->ClientSocket,
		&ContextObject->ReceiveWsaInBuffer,                //�������ݵ��ڴ�          
		1,
		&ReternLength,
		&Flags,
		&OverlappedEx->m_Overlapped,
		NULL);
	if (IsOk == SOCKET_ERROR&&WSAGetLastError() != WSA_IO_PENDING)
	{
		//����������
		RemoveContextObject(ContextObject);
	}
}
PCONTEXT_OBJECT  _CIOCPServer::AllocateContextObiect()
{
	PCONTEXT_OBJECT   ContextObject = NULL;
	//�����ٽ���
	_CCriticalSection  CriticalSection(m_CriticalSection);
	//�ж��ڴ���Ƿ�Ϊ��
	if (m_FreeContextObjectList.IsEmpty() == FALSE)
	{
		//�ڴ��ȡ�ڴ�
		ContextObject = m_FreeContextObjectList.RemoveHead();
	}
	else
	{
		ContextObject = new  CONTEXT_OBJECT;
	}
	if (ContextObject != NULL)
	{
		//��ʼ����Ա����
		ContextObject->InitMember();
	}
	return ContextObject;
}
void  _CIOCPServer::RemoveContextObject(CONTEXT_OBJECT*  ContextObject)
{
	_CCriticalSection  CriticalSection(m_CriticalSection);
	//���ڴ��в��Ҹ��û������±��������ݽṹ
	if (m_ConnectionContextObjectList.Find(ContextObject))
	{
		//ȡ���ڵ�ǰ�׽��ֵ��첽IO��ǰ��δ��ɵ��첽����ȫ������ȡ��
		CancelIo((HANDLE)ContextObject->ClientSocket);
		//�ر��׽���
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = INVALID_SOCKET;
		//�жϻ���û���첽IO�����ڵ�ǰ�׽�����
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)ContextObject))
		{
			Sleep(0);
		}
		//�����ڴ�ṹ�������ڴ��
		MoveContextObjectToFreePool(ContextObject);

	}
}
void  _CIOCPServer::MoveContextObjectToFreePool(CONTEXT_OBJECT*  ContextObject)
{
	_CCriticalSection  CriticalSection(m_CriticalSection);

	POSITION  Position = m_ConnectionContextObjectList.Find(ContextObject);
	if (Position)
	{
		ContextObject->m_ReceivedCompressedBufferData.ClearArray();
		ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
		ContextObject->m_SendCompressedBufferData.ClearArray();
		memset(ContextObject->m_BufferData, 0, PACKET_LENGTH);
		m_FreeContextObjectList.AddTail(ContextObject);
		m_ConnectionContextObjectList.RemoveAt(Position);
	}
}
BOOL     _CIOCPServer::InitializeIOCP()
{
	//������ɶ˿�
	m_CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPortHandle == NULL)
	{
		return FALSE;
	}
	SYSTEM_INFO   SystemInfo;
	//��õ����м���
	GetSystemInfo(&SystemInfo);
	m_ThreadPoolMin = 1;
	m_ThreadPoolMax = SystemInfo.dwNumberOfProcessors * 2;

	ULONG  WorkThreadCount = 2;
	HANDLE  WorkThreadHandle = NULL;
	for (int i = 0; i < WorkThreadCount; i++)
	{
		WorkThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThreadProcedure,
			(void*)this, 0, NULL);
		if (WorkThreadHandle == NULL)
		{
			CloseHandle(m_CompletionPortHandle);
			return  FALSE;
		}
		m_WorkThreadCount++;
		CloseHandle(WorkThreadHandle);
	}
	return TRUE;
}
DWORD WINAPI   _CIOCPServer::WorkThreadProcedure(LPVOID  ParameterData)
{
	_CIOCPServer* This = (_CIOCPServer*)ParameterData;
	HANDLE  CompletionPortHandle = This->m_CompletionPortHandle;
	DWORD   TransforDataLength = 0;
	PCONTEXT_OBJECT  ContextObject = NULL;
	LPOVERLAPPED   OverLapped = NULL;
	OVERLAPPEDEX*  OverLappedEx = NULL;

	ULONG         BusyThread = 0;
	BOOL          v1 = FALSE;

	InterlockedIncrement(&This->m_CurrentThreadCount);
	InterlockedIncrement(&This->m_BusyThreadCount);

	while (This->m_IsWorking == FALSE)
	{
		InterlockedDecrement(&This->m_BusyThreadCount);

		//�ú�����������������ú�������˵��������õ������
		BOOL IsOk = GetQueuedCompletionStatus(
			CompletionPortHandle,
			&TransforDataLength,              //��ɶ�������
			(PULONG_PTR)&ContextObject,       //���Key
			&OverLapped,
			60000);

		DWORD LastError = GetLastError();
		OverLappedEx = CONTAINING_RECORD(OverLapped, OVERLAPPEDEX, m_Overlapped);

		BusyThread = InterlockedIncrement(&This->m_BusyThreadCount);
		if (!IsOk&&LastError != WAIT_TIMEOUT)
		{
			if (ContextObject&&This->m_IsWorking == FALSE&&TransforDataLength == 0)
			{
				//���Է����׻��Ʒ����ر�
				This->RemoveContextObject(ContextObject);
			}
			continue;
		}
		if (!v1)
		{
			//����һ���µ��̵߳��̳߳�
			if (BusyThread == This->m_CurrentThreadCount)
			{
				if (BusyThread < This->m_ThreadPoolMax)
				{
					if (ContextObject != NULL)
					{
						HANDLE  ThreadHandle = (HANDLE)CreateThread(NULL,
							0,
							(LPTHREAD_START_ROUTINE)WorkThreadProcedure,
							(void*)This,
							0,
							NULL);
						InterlockedIncrement(&This->m_WorkThreadCount);
						CloseHandle(ThreadHandle);
					}
				}
			}
			//����һ���̴߳��̳߳�
			if (!IsOk&&LastError == WAIT_TIMEOUT)
			{
				if (ContextObject == NULL)
				{
					if (This->m_CurrentThreadCount > This->m_ThreadPoolMin)
					{
						break;
					}
					v1 = TRUE;
				}
			}
		}
		if (!v1)
		{
			//����õ���ɣ��������� IO_INITIALIZE �� IO_RECEIVE��
			if (IsOk&&OverLappedEx != NULL&&ContextObject != NULL)
			{
				try 
				{
					//����֮ǰͶ�ݵ�û����ɵ�����
					This->HandleIO(OverLappedEx->m_PackType, ContextObject, TransforDataLength);
					//û���ͷ��ڴ�
					ContextObject = NULL;
				}
				catch (...) {}
			}
		}
		if (OverLappedEx)
		{
			delete  OverLappedEx;
			OverLappedEx = NULL;
		}

	}
	InterlockedDecrement(&This->m_WorkThreadCount);
	InterlockedDecrement(&This->m_CurrentThreadCount);
	InterlockedDecrement(&This->m_BusyThreadCount);
	return 0;
}
BOOL     _CIOCPServer::HandleIO(PACKET_TYPE PacketType, PCONTEXT_OBJECT  ContextObject, DWORD TransforDataLength)
{
	BOOL v1 = FALSE;
	if (IO_INITIALIZE == PacketType)
	{
	//	v1 = OnInitializing(ContextObject, TransforDataLength);
	}
	if (IO_RECEIVE == PacketType)
	{
		v1 = OnReceiving(ContextObject, TransforDataLength);
	}
	if (IO_SEND == PacketType)
	{
		//�������ݵ��ͻ���
		v1 = OnPostSending(ContextObject, TransforDataLength);
	}
	return  v1;
}
BOOL     _CIOCPServer::OnInitializing(PCONTEXT_OBJECT  ContextObject, DWORD TransforDataLength)
{
	MessageBox(NULL, "OnInitializing", "OnInitializing", 0);
	return  TRUE;
}
BOOL     _CIOCPServer::OnReceiving(PCONTEXT_OBJECT  ContextObject, DWORD TransforDataLength)
{
	_CCriticalSection  CriticalSection(m_CriticalSection);
	try
	{
		if (TransforDataLength == 0)
		{
			//�Է��ر����׽���
			MessageBox(NULL, "�ر��׽���", "�ر��׽���",0);
			RemoveContextObject(ContextObject);
			return FALSE;
		}
		//���ӵ������ݿ�����m_ReceivedCompressedBufferData
		ContextObject->m_ReceivedCompressedBufferData.WriteArray(
			(PBYTE)ContextObject->m_BufferData, TransforDataLength);
		//��ȡ���ݰ���ͷ�������ݰ���ͷ��������ѹ����
		while (ContextObject->m_ReceivedCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)
		{
			//�洢���ݰ�ͷ����־
			char  v1[PACKET_FLAG_LENGTH] = { 0 };
			//�������ݰ�ͷ����־
			CopyMemory(v1, ContextObject->m_ReceivedCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);
			//У�����ݰ�ͷ����־
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGTH) != 0)
			{
				throw"Bad Buffer";
			}
			//��ȡ���ݰ��ܴ�С
			ULONG   PackTotalLength = 0;
			CopyMemory(&PackTotalLength, 
				ContextObject->m_ReceivedCompressedBufferData.GetArray(PACKET_FLAG_LENGTH), sizeof(ULONG));

			int a = ContextObject->m_ReceivedCompressedBufferData.GetArrayLength();
			if (PackTotalLength && a >= PackTotalLength)
			{
				//��ʼ���ݰ�  [shine][ѹ���ĳ���+13][û��ѹ���ĳ���][HelloWorld..]
				ULONG  DecompressedLength = 0;               //û��ѹ���ĳ���

				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)v1, PACKET_FLAG_LENGTH);
				//[ѹ���ĳ���+13][û��ѹ���ĳ���][HelloWorld..]

				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)&PackTotalLength, sizeof(ULONG));
				//[û��ѹ���ĳ���][HelloWorld..]

				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)&DecompressedLength, sizeof(ULONG));
				//[HelloWorld..]

				ULONG  CompressedLength = PackTotalLength - PACKET_HEADER_LENGTH;
				//ѹ������
				PBYTE  CompressedData = new BYTE[CompressedLength];               //û�н�ѹ���ݳ���
				//��ѹ����
				PBYTE  DeCompressedData = new  BYTE[DecompressedLength];
				if (CompressedData == NULL || DeCompressedData == NULL)
				{
					throw" Bad Allocate";
				}
				//�����ݰ��ж�ȡѹ���������
				ContextObject->m_ReceivedCompressedBufferData.ReadArray(CompressedData, CompressedLength);
				//��ѹ��
				int IsOk = uncompress(DeCompressedData, &DecompressedLength, CompressedData, CompressedLength);
				if (IsOk == Z_OK)
				{
					ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
					ContextObject->m_ReceivedCompressedBufferData.ClearArray();
					//������ʵ����
					ContextObject->m_ReceivedDecompressedBufferData.WriteArray(DeCompressedData, DecompressedLength);
					delete[] CompressedData;
					delete[] DeCompressedData;

					m_WindowNotifyProcedure(ContextObject);

				}
				else
				{
					delete[] CompressedData;
					delete[] DeCompressedData;
					throw"Bad Buffer";
				}
			}
			else
			{
				break;
			}
		}
		//��һ�ε��첽�����Ѿ���ɣ�����Ͷ���µ��첽����
		PostRecv(ContextObject);
	}
	catch (...)
	{
		ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
		ContextObject->m_ReceivedCompressedBufferData.ClearArray();
		PostRecv(ContextObject);
	}
	return TRUE;
}
void     _CIOCPServer::OnPreSending(PCONTEXT_OBJECT  ContextObject, PBYTE  BufferData, ULONG BufferLength)
{
	if (ContextObject == NULL)
	{
		return;
	}
	try
	{
		if (BufferLength > 0)
		{
			unsigned  long  ComprssedLength = (double)BufferLength*1.001 + 12;
			LPBYTE          CompressedData = new  BYTE[ComprssedLength];
			if (CompressedData == NULL)
			{
				return;
			}
			int  IsOk = compress(CompressedData, &ComprssedLength, (LPBYTE)BufferData, BufferLength);
			//��ѹ��ʧ��
			if (IsOk != Z_OK)
			{
				delete[]  CompressedData;
				return;
			}
			//�������ݰ��ܳ�
			ULONG  PackTotalLength = ComprssedLength + PACKET_HEADER_LENGTH;
			//�����ݰ���д������

			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)m_PacketHeaderFlag, sizeof(m_PacketHeaderFlag));
			//[Shine]

			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)&PackTotalLength, sizeof(ULONG));
			//[Shine][PackTotalLength]

			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)&BufferLength, sizeof(ULONG));
			//[Shine][PackTotalLength][BufferLength]

			ContextObject->m_SendCompressedBufferData.WriteArray(CompressedData, ComprssedLength);
			//[Shine][PackTotalLength][BufferLength][ѹ�������ʵ����]

			delete[]  CompressedData;
			CompressedData = NULL;
		}
		OVERLAPPEDEX*  OverlappedEx = new OVERLAPPEDEX(IO_SEND);
		PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (DWORD)ContextObject, &OverlappedEx->m_Overlapped);

	}
	catch (...) {}
}
BOOL     _CIOCPServer::OnPostSending(PCONTEXT_OBJECT  ContextObject, DWORD TransforDataLength)
{
	try
	{
		DWORD  Flags = MSG_PARTIAL;
		//����ɵ����ݴ����ݽṹ��ȥ��
		ContextObject->m_SendCompressedBufferData.RemoveComletedArray(TransforDataLength);
		//�жϻ��ж���������Ҫ����
		if (ContextObject->m_SendCompressedBufferData.GetArrayLength() == 0)
		{
			//�����Ѿ��������
			ContextObject->m_SendCompressedBufferData.ClearArray();
			return TRUE;
		}
		else
		{
			//������������
			OVERLAPPEDEX* OverlappedEx = new  OVERLAPPEDEX(IO_SEND);
			ContextObject->SendWsaOutBuffer.buf = (char*)ContextObject->m_SendCompressedBufferData.GetArray();
			ContextObject->SendWsaOutBuffer.len = ContextObject->m_SendCompressedBufferData.GetArrayLength();

			int IsOk = WSASend(ContextObject->ClientSocket,
				&ContextObject->SendWsaOutBuffer,
				1,
				&ContextObject->SendWsaOutBuffer.len,
				Flags,
				&OverlappedEx->m_Overlapped,
				NULL);
			if (IsOk == SOCKET_ERROR&&WSAGetLastError() != WSA_IO_PENDING)
			{
				RemoveContextObject(ContextObject);
			}
		}
	}
	catch (...) {}
	return  FALSE;
}
