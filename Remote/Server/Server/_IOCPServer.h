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
	SOCKET  ClientSocket;              //Serverʹ�ø��׽�����Ŀ��Client����ͨѶ
	WSABUF  ReceiveWsaInBuffer;        //�����û���������ݣ��ýṹ����û���ڴ棬��m_BufferData����
	WSABUF  SendWsaOutBuffer;        //��ͻ����͵����ݣ��ýṹ����û���ڴ棬��m_SendCompressedBufferData����
	char    m_BufferData[PACKET_LENGTH];
	_CArray  m_ReceivedCompressedBufferData;              //���յ���ѹ��������m_BufferData�������ó�Ա��
	_CArray  m_ReceivedDecompressedBufferData;              //���յ���ѹ�������ݽ��н�ѹ�������
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
//����һ����������
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
	//Ͷ�����������ݵ��첽����
	void     PostRecv(CONTEXT_OBJECT*  ContextObject);
	//�����õ��˿ͻ��˵�����
	BOOL     OnReceiving(PCONTEXT_OBJECT  ContextObject, DWORD TransforDataLength);
	//�����ݷ��͵��ͻ���
	BOOL     OnPostSending(PCONTEXT_OBJECT  ContextObject, DWORD TransforDataLength);
	//�������ݰ�
	void     OnPreSending(PCONTEXT_OBJECT  ContextObject, PBYTE  BufferData, ULONG BufferLength);
private:
	SOCKET   m_ListenSocket;                       //�����׽���
	HANDLE   m_ListenThreadHandle;                //�����߳̾��
	HANDLE   m_ListenEventHandle;                 //�����¼�
	CRITICAL_SECTION   m_CriticalSection;         //�ؼ������
	HANDLE   m_CompletionPortHandle;             //��ɶ˿ھ��
	HANDLE   m_KillEventHandle;                 //�����߳��˳�ѭ��
	BOOL     m_IsWorking;                       //�����̵߳�ѭ����־
	CONTEXT_OBJECT_LIST  m_FreeContextObjectList ;        //�ڴ��ģ��
	CONTEXT_OBJECT_LIST  m_ConnectionContextObjectList;   //�����û�
	long    m_KeepAliveTime;                             //�������
	ULONG   m_ThreadPoolMin;
	ULONG   m_ThreadPoolMax;
	ULONG   m_WorkThreadCount;
	ULONG   m_CurrentThreadCount;
	ULONG   m_BusyThreadCount;		
	char    m_PacketHeaderFlag[PACKET_FLAG_LENGTH];               //���ݰ�ͷ����־
	LPFNWINDOWNOTIFYPROCEDURE  m_WindowNotifyProcedure;           //�����ɴ������д��ݹ����ĺ���ָ��
};

