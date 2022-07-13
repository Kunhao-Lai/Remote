#pragma once
#include<winsock2.h>
#include<mstcpip.h>
#include<windows.h>
#include"_Array.h"
#include"zconf.h"
using namespace std;
#include"zlib.h"
#pragma  comment(lib,"WS2_32.lib")
#include"Manager.h"

#define  PACKET_LENGTH  0X2000
#define  PACKET_FLAG_LENGTH        5
#define  PACKET_HEADER_LENGTH      13
#define  MAX_SEND_BUFFER       0x2000

class _CIOCPClient
{
public:
	_CIOCPClient();
	~_CIOCPClient();
	HANDLE   m_EventHandle;                    //事件
	BOOL  ConnectServer(char* ServerIPAddress, USHORT  ServerConnectPort);
	static  DWORD WINAPI  WorkThreadProcedure(LPVOID  ParameterData);
	BOOL   IsReceiving() { return m_IsReceiving; }
	void   Disconnect();
	int    OnSending(char* BufferData, ULONG  BufferLength);
	BOOL   SendWithSplit(char* BufferData, ULONG BufferLength, ULONG SplitLength);
	void   OnReceiving(char* BufferData, ULONG  BufferLength);
	void   SetManagerObject(class CManager*  Manager)
	{
		m_ManagerObject = Manager;
	}
	void   WaitForEvent()
	{
		//避免对象销毁
		WaitForSingleObject(m_EventHandle, INFINITE);
	}
public:
	SOCKET   m_ClientSocket;                       //与服务器进行连接的套接字
	HANDLE   m_WorkThreadHandle;                //工作线程句柄
	
	BOOL      m_IsReceiving;
	char      m_PacketHeaderFlag[PACKET_FLAG_LENGTH];               //数据包头部标志
	_CArray  m_ReceivedCompressedBufferData;              //接收到的压缩的数据m_BufferData拷贝到该成员中
	_CArray  m_ReceivedDecompressedBufferData;              //接收到的压缩的数据进行解压后的数据
	_CArray  m_SendCompressedBufferData;
	CManager*  m_ManagerObject;
};

