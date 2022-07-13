#include "stdafx.h"
#include "_IOCPServer.h"


_CIOCPServer::_CIOCPServer()
{
	//初始化套接字类库
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
	//出发事件使其正常退出监听线程的循环
	SetEvent(m_KillEventHandle);
	//等待监听线程退出
	//SetEvent(m_ListenEventHandle);
	WaitForSingleObject(m_ListenThreadHandle, INFINITE);

	//回收资源
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

	//销毁关键代码段
	DeleteCriticalSection(&m_CriticalSection);
	WSACleanup();

}
BOOL   _CIOCPServer::ServerRun(USHORT  ListenPort, LPFNWINDOWNOTIFYPROCEDURE  WindowNotifyProcedure)
{		
	m_WindowNotifyProcedure = WindowNotifyProcedure;
	//创建退出监听线程的事件
	m_KillEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_KillEventHandle == NULL)
	{
		return  FALSE;
	}
	//创建一个监听套接字
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_ListenSocket == INVALID_SOCKET)
	{
		return  FALSE;
	}
	//创建一个监听事件
	m_ListenEventHandle = WSACreateEvent();
	DWORD LastError = GetLastError();
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return  FALSE;
	}
	//将监听套接字与事件进行关联
	BOOL  IsOk = WSAEventSelect(m_ListenSocket, m_ListenEventHandle, FD_ACCEPT | FD_CLOSE);
	if (IsOk == SOCKET_ERROR)
	{
		goto  Error;
	}
	//初始化Server端网卡
	SOCKADDR_IN  ServerAddress;
	ServerAddress.sin_port = htons(ListenPort);
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	//绑定套接字
	IsOk = bind(m_ListenSocket, (sockaddr*)&ServerAddress,sizeof(ServerAddress));
	if (IsOk == SOCKET_ERROR)
	{
		goto  Error;
	}
	//监听
	IsOk = listen(m_ListenSocket, SOMAXCONN);
	if (IsOk == SOCKET_ERROR)
	{
		goto  Error;
	}
	//创建监听线程
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
	
	//初始化IOCP
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
			//由于析构函数触发m_KillEventHandle事件
			break;
		}
		DWORD v1;
		//等待监听事件受信（监听套接字受信）
		v1 = WSAWaitForMultipleEvents(1,
			&This->m_ListenEventHandle,
			FALSE,
			100,
			FALSE);
		DWORD LastError = GetLastError();
		if (v1 == WSA_WAIT_TIMEOUT)
		{
			//该事件没有受信
			continue;
		}
		//如果事件受信我们就将该事件转换成一个网络事件进行判断
		v1 = WSAEnumNetworkEvents(This->m_ListenSocket,
			This->m_ListenEventHandle,
			&NetWorkEvents);
		if (v1 == SOCKET_ERROR)
		{
			break;
		}
		if (NetWorkEvents.lNetworkEvents&FD_ACCEPT)  //监听套接字受信
		{
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				//将客户端上线请求处理
				This->OnAccept();
			}
			else
			{
				break;
			}
		}
		else
		{
			//当删除一个用户时，代码会执行到这里
		//	break;
		}
	}
	return 0;
}
void    _CIOCPServer::OnAccept()
{
	int  Result = 0;
	//保存上线用户IP地址
	SOCKET  ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN  ClientAddress = { 0 };
	int    ClientAddressLength = sizeof(SOCKADDR_IN);

	ClientSocket = accept(m_ListenSocket,          //同步接收上线信息
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

	//成员赋值
	ContextObject->ClientSocket = ClientSocket;
	//关联内存（wsaRecv用于接收客户端来的数据）
	ContextObject->ReceiveWsaInBuffer.buf = (char*)ContextObject->m_BufferData;
	ContextObject->ReceiveWsaInBuffer.len = sizeof(ContextObject->m_BufferData);

	//将生成的通讯套接字与完成端口句柄相关联
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

	//保持连接，检测双方主机是否崩溃，若两小时内在此套接口的任一方向都没有数据交换
	//TCP自动给对方发一个保持存活

	//设置套接字的选项卡Set KeepAlive 开启保活机制 SO_KEEPALIVE
	m_KeepAliveTime = 3;
	BOOL  IsOk = TRUE;

	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsOk, sizeof(IsOk)) != 0)
	{

	}
	
	//设置超时详细信息
	tcp_keepalive  KeepAlive;
	KeepAlive.onoff = 1;                                 //启用保活
	KeepAlive.keepalivetime = m_KeepAliveTime;           //超过三分钟没有数据，就发送探测包
	KeepAlive.keepaliveinterval = 1000 * 10;             //重试间隔为10秒

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
	//向完成端口投递一个请求
	//工作线程会等待完成端口的完成状态
	IsOk = PostQueuedCompletionStatus(m_CompletionPortHandle,
		0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);
	//如果投递失败
	if ((!IsOk&&GetLastError() != ERROR_IO_PENDING))
	{
		RemoveContextObject(ContextObject);
		return;
	}
	//该上线用户已完成了上线的请求
	//服务器向该用户投递PostRecv请求
	PostRecv(ContextObject);
}
void  _CIOCPServer::PostRecv(CONTEXT_OBJECT*  ContextObject)
{
	//向我们刚上线的用户投递一个接收数据的请求
	//如果用户的第一个数据包到达即被控端的登陆请求到达我们的工作线程就会响应并调用ProcessIOMessage函数
	OVERLAPPEDEX*  OverlappedEx = new  OVERLAPPEDEX(IO_RECEIVE);

	DWORD  ReternLength;
	ULONG  Flags = MSG_PARTIAL;
	int    IsOk = WSARecv(ContextObject->ClientSocket,
		&ContextObject->ReceiveWsaInBuffer,                //接收数据的内存          
		1,
		&ReternLength,
		&Flags,
		&OverlappedEx->m_Overlapped,
		NULL);
	if (IsOk == SOCKET_ERROR&&WSAGetLastError() != WSA_IO_PENDING)
	{
		//请求发生错误
		RemoveContextObject(ContextObject);
	}
}
PCONTEXT_OBJECT  _CIOCPServer::AllocateContextObiect()
{
	PCONTEXT_OBJECT   ContextObject = NULL;
	//进入临界区
	_CCriticalSection  CriticalSection(m_CriticalSection);
	//判断内存池是否为空
	if (m_FreeContextObjectList.IsEmpty() == FALSE)
	{
		//内存池取内存
		ContextObject = m_FreeContextObjectList.RemoveHead();
	}
	else
	{
		ContextObject = new  CONTEXT_OBJECT;
	}
	if (ContextObject != NULL)
	{
		//初始化成员变量
		ContextObject->InitMember();
	}
	return ContextObject;
}
void  _CIOCPServer::RemoveContextObject(CONTEXT_OBJECT*  ContextObject)
{
	_CCriticalSection  CriticalSection(m_CriticalSection);
	//在内存中查找该用户的上下背景文数据结构
	if (m_ConnectionContextObjectList.Find(ContextObject))
	{
		//取消在当前套接字的异步IO以前的未完成的异步请求全部立即取消
		CancelIo((HANDLE)ContextObject->ClientSocket);
		//关闭套接字
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = INVALID_SOCKET;
		//判断还有没有异步IO请求在当前套接字上
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)ContextObject))
		{
			Sleep(0);
		}
		//将该内存结构回收至内存池
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
	//创建完成端口
	m_CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPortHandle == NULL)
	{
		return FALSE;
	}
	SYSTEM_INFO   SystemInfo;
	//获得电脑有几核
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

		//该函数是阻塞函数如果该函数返回说明有请求得到了完成
		BOOL IsOk = GetQueuedCompletionStatus(
			CompletionPortHandle,
			&TransforDataLength,              //完成多少数据
			(PULONG_PTR)&ContextObject,       //完成Key
			&OverLapped,
			60000);

		DWORD LastError = GetLastError();
		OverLappedEx = CONTAINING_RECORD(OverLapped, OVERLAPPEDEX, m_Overlapped);

		BusyThread = InterlockedIncrement(&This->m_BusyThreadCount);
		if (!IsOk&&LastError != WAIT_TIMEOUT)
		{
			if (ContextObject&&This->m_IsWorking == FALSE&&TransforDataLength == 0)
			{
				//当对方的套机制发生关闭
				This->RemoveContextObject(ContextObject);
			}
			continue;
		}
		if (!v1)
		{
			//分配一个新的线程到线程池
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
			//销毁一个线程从线程池
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
			//请求得到完成（两种请求 IO_INITIALIZE 和 IO_RECEIVE）
			if (IsOk&&OverLappedEx != NULL&&ContextObject != NULL)
			{
				try 
				{
					//处理之前投递的没有完成的请求
					This->HandleIO(OverLappedEx->m_PackType, ContextObject, TransforDataLength);
					//没有释放内存
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
		//发送数据到客户端
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
			//对方关闭了套接字
			MessageBox(NULL, "关闭套接字", "关闭套接字",0);
			RemoveContextObject(ContextObject);
			return FALSE;
		}
		//将接到的数据拷贝到m_ReceivedCompressedBufferData
		ContextObject->m_ReceivedCompressedBufferData.WriteArray(
			(PBYTE)ContextObject->m_BufferData, TransforDataLength);
		//读取数据包的头部（数据包的头部不参与压缩）
		while (ContextObject->m_ReceivedCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)
		{
			//存储数据包头部标志
			char  v1[PACKET_FLAG_LENGTH] = { 0 };
			//拷贝数据包头部标志
			CopyMemory(v1, ContextObject->m_ReceivedCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);
			//校验数据包头部标志
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGTH) != 0)
			{
				throw"Bad Buffer";
			}
			//获取数据包总大小
			ULONG   PackTotalLength = 0;
			CopyMemory(&PackTotalLength, 
				ContextObject->m_ReceivedCompressedBufferData.GetArray(PACKET_FLAG_LENGTH), sizeof(ULONG));

			int a = ContextObject->m_ReceivedCompressedBufferData.GetArrayLength();
			if (PackTotalLength && a >= PackTotalLength)
			{
				//初始数据包  [shine][压缩的长度+13][没有压缩的长度][HelloWorld..]
				ULONG  DecompressedLength = 0;               //没有压缩的长度

				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)v1, PACKET_FLAG_LENGTH);
				//[压缩的长度+13][没有压缩的长度][HelloWorld..]

				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)&PackTotalLength, sizeof(ULONG));
				//[没有压缩的长度][HelloWorld..]

				ContextObject->m_ReceivedCompressedBufferData.ReadArray((PBYTE)&DecompressedLength, sizeof(ULONG));
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
				ContextObject->m_ReceivedCompressedBufferData.ReadArray(CompressedData, CompressedLength);
				//解压缩
				int IsOk = uncompress(DeCompressedData, &DecompressedLength, CompressedData, CompressedLength);
				if (IsOk == Z_OK)
				{
					ContextObject->m_ReceivedDecompressedBufferData.ClearArray();
					ContextObject->m_ReceivedCompressedBufferData.ClearArray();
					//拷贝真实数据
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
		//上一次的异步请求已经完成，重新投递新的异步请求
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
			//若压缩失败
			if (IsOk != Z_OK)
			{
				delete[]  CompressedData;
				return;
			}
			//计算数据包总长
			ULONG  PackTotalLength = ComprssedLength + PACKET_HEADER_LENGTH;
			//向数据包中写入数据

			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)m_PacketHeaderFlag, sizeof(m_PacketHeaderFlag));
			//[Shine]

			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)&PackTotalLength, sizeof(ULONG));
			//[Shine][PackTotalLength]

			ContextObject->m_SendCompressedBufferData.WriteArray((PBYTE)&BufferLength, sizeof(ULONG));
			//[Shine][PackTotalLength][BufferLength]

			ContextObject->m_SendCompressedBufferData.WriteArray(CompressedData, ComprssedLength);
			//[Shine][PackTotalLength][BufferLength][压缩后的真实数据]

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
		//将完成的数据从数据结构中去除
		ContextObject->m_SendCompressedBufferData.RemoveComletedArray(TransforDataLength);
		//判断还有多少数据需要发送
		if (ContextObject->m_SendCompressedBufferData.GetArrayLength() == 0)
		{
			//数据已经发送完毕
			ContextObject->m_SendCompressedBufferData.ClearArray();
			return TRUE;
		}
		else
		{
			//真正发送数据
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
