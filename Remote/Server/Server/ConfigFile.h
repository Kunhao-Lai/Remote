#pragma once
#include<iostream>
#include<windows.h>
using namespace std;
class CConfigFile
{
public:
	CConfigFile();
	~CConfigFile();
	BOOL  InitConfigFile();
	int   GetInt(string  MainKey, string  SubKey);
	BOOL  SetInt(string  MainKey, string SubKey, int BufferData);
private:
	string  m_FileFullPath;
};

