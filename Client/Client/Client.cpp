#include "Client.h"


struct  _SERVER_CONNECT_INFORMATION_
{
	DWORD   CheckFlag;
	char    ServerIPAddress[20];
	USHORT  ServerConnectPort;
}__ServerConnectInfo = { 0x87654321,"127.0.0.1",2356 };

void main()
{
	printf("%s\r\n", __ServerConnectInfo.ServerIPAddress);

	//����Dll.dllģ��
	HMODULE  ModuleHandle = (HMODULE)LoadLibrary("Dll.dll");
	if (ModuleHandle == NULL)
	{
		return;
	}
	//��ȡһ��Dllģ���е�һ����������
	LPFN_CLIENTRUN  ClientRun =
		(LPFN_CLIENTRUN)GetProcAddress(ModuleHandle, "ClientRun");
	if (ClientRun == NULL)
	{
		FreeLibrary(ModuleHandle);
		return;
	}
	else 
	{
		ClientRun(__ServerConnectInfo.ServerIPAddress, __ServerConnectInfo.ServerConnectPort);
	}
	printf("Input  AnyKey To Exit\r\n");
	getchar();
	FreeLibrary(ModuleHandle);
}