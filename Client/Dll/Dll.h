#pragma once
#include"_IOCPClient.h"


void  ClientRun(char* ServerIPAddres, USHORT  ServerConnectPort);
DWORD WINAPI  WorkThreadProcedure(LPVOID ParameterData);