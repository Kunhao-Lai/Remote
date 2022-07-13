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
	HANDLE   m_EventHandle;                    //�¼�
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
		//�����������
		WaitForSingleObject(m_EventHandle, INFINITE);
	}
public:
	SOCKET   m_ClientSocket;                       //��������������ӵ��׽���
	HANDLE   m_WorkThreadHandle;                //�����߳̾��
	
	BOOL      m_IsReceiving;
	char      m_PacketHeaderFlag[PACKET_FLAG_LENGTH];               //���ݰ�ͷ����־
	_CArray  m_ReceivedCompressedBufferData;              //���յ���ѹ��������m_BufferData�������ó�Ա��
	_CArray  m_ReceivedDecompressedBufferData;              //���յ���ѹ�������ݽ��н�ѹ�������
	_CArray  m_SendCompressedBufferData;
	CManager*  m_ManagerObject;
};

