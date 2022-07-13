#pragma once
#include<winsock2.h>
#include<mstcpip.h>
#include<windows.h>
#include<iostream>
#include"Common.h"
#include"_CriticalSection.h"
#include"_Array.h"
#include"zconf.h"
#include"zlib.h"
using namespace std;
#pragma  comment(lib,"WS2_32.lib")

#define  PACKET_LENGTH    0X2000
#define  MAX_RECV_BUFFER  0x2000
#define  MAX_SEND_BUFFER  0x2000
#define  PACKET_FLAG_LENGTH        5
#define  PACKET_HEADER_LENGTH      13
typedef struct _CONTEXT_OBJECT_
{
	SOCKET  ClientSocket;              //Server使用该套接字与目标Client进行通讯
	WSABUF  ReceiveWsaInBuffer;        //接收用户到达的数据，该结构本身没有内存，与m_BufferData关联
	WSABUF  SendWsaOutBuffer;        //向客户发送的数据，该结构本身没有内存，与m_SendCompressedBufferData关联
	char    m_BufferData[PACKET_LENGTH];
	_CArray  m_ReceivedCompressedBufferData;              //接收到的压缩的数据m_BufferData拷贝到该成员中
	_CArray  m_ReceivedDecompressedBufferData;              //接收到的压缩的数据进行解压后的数据
	_CArray  m_SendCompressedBufferData;
	int       DialogID;
	HANDLE    DialogHandle;
	void  InitMember()
	{
		ClientSocket = INVALID_SOCKET;
		memset(m_BufferData, 0, sizeof(char)*PACKET_LENGTH);
		memset(&ReceiveWsaInBuffer, 0, sizeof(WSABUF));
		memset(&SendWsaOutBuffer, 0, sizeof(WSABUF));
		DialogHandle = NULL;
		DialogID = 0;
	}
	
}CONTEXT_OBJECT, *PCONTEXT_OBJECT;

enum  PACKET_TYPE
{
	IO_INITIALIZE,
	IO_RECEIVE,
	IO_SEND,
	IO_IDLE
};

class  OVERLAPPEDEX
{
public:
	OVERLAPPED   m_Overlapped;
	PACKET_TYPE  m_PackType;
	OVERLAPPEDEX(PACKET_TYPE  PacketType)
	{
		ZeroMemory(this, sizeof(OVERLAPPEDEX));
		m_PackType = PacketType;
	}
};

typedef  void  (CALLBACK *LPFNWINDOWNOTIFYPROCEDURE)(PCONTEXT_OBJECT  ContextObject);
//定义一个数据类型
typedef CList<PCONTEXT_OBJECT>  CONTEXT_OBJECT_LIST;
class _CIOCPServer
{
public:
	_CIOCPServer();
	~_CIOCPServer();
	BOOL   ServerRun(USHORT  ListenPort, LPFNWINDOWNOTIFYPROCEDURE  WindowNotifyProcedure);
	static  DWORD WINAPI  ListenThreadProcedure(LPVOID  ParameterData);
	static  DWORD WINAPI  WorkThreadProcedure(LPVOID  ParameterData);
	void   OnAccept();
	PCONTEXT_OBJECT  AllocateContextObiect();
	void     RemoveContextObject(CONTEXT_OBJECT*  ContextObject);
	void     MoveContextObjectToFreePool(CONTEXT_OBJECT*  ContextObject);
	BOOL     InitializeIOCP();
	BOOL     HandleIO(PACKET_TYPE PacketType, PCONTEXT_OBJECT  ContextObject, DWORD TransforDataLength);
	BOOL     OnInitializing(PCONTEXT_OBJECT  ContextObject, DWORD TransforDataLength);
	//投递了请求数据的异步请求
	void     PostRecv(CONTEXT_OBJECT*  ContextObject);
	//真正得到了客户端的数据
	BOOL     OnReceiving(PCONTEXT_OBJECT  ContextObject, DWORD TransforDataLength);
	//将数据发送到客户端
	BOOL     OnPostSending(PCONTEXT_OBJECT  ContextObject, DWORD TransforDataLength);
	//构建数据包
	void     OnPreSending(PCONTEXT_OBJECT  ContextObject, PBYTE  BufferData, ULONG BufferLength);
private:
	SOCKET   m_ListenSocket;                       //监听套接字
	HANDLE   m_ListenThreadHandle;                //监听线程句柄
	HANDLE   m_ListenEventHandle;                 //监听事件
	CRITICAL_SECTION   m_CriticalSection;         //关键代码段
	HANDLE   m_CompletionPortHandle;             //完成端口句柄
	HANDLE   m_KillEventHandle;                 //监听线程退出循环
	BOOL     m_IsWorking;                       //工作线程的循环标志
	CONTEXT_OBJECT_LIST  m_FreeContextObjectList ;        //内存池模块
	CONTEXT_OBJECT_LIST  m_ConnectionContextObjectList;   //上线用户
	long    m_KeepAliveTime;                             //保活机制
	ULONG   m_ThreadPoolMin;
	ULONG   m_ThreadPoolMax;
	ULONG   m_WorkThreadCount;
	ULONG   m_CurrentThreadCount;
	ULONG   m_BusyThreadCount;		
	char    m_PacketHeaderFlag[PACKET_FLAG_LENGTH];               //数据包头部标志
	LPFNWINDOWNOTIFYPROCEDURE  m_WindowNotifyProcedure;           //保存由窗口类中传递过来的函数指针
};

