#pragma once
#include<windows.h>
#include"iostream"
using namespace std;
class CScreenSpy
{
public:
	CScreenSpy();
	CScreenSpy(ULONG  BitmapCount);
	~CScreenSpy();
	LPBITMAPINFO  OnInitBitmapInfo(ULONG BitmapCount, ULONG FullWidth, ULONG FullHeigth);
	//获得位图信息
	ULONG  GetBitmapInfoSize();
	LPBITMAPINFO  GetBitmapInfoData();
	//抓图
	LPVOID  GetFirstScreenData();
	ULONG  GetFirstScreenSize();
	LPVOID  GetNextScreenData(ULONG* BufferLength);
	void    WriteScreenData(LPBYTE  BufferData, ULONG  BufferLength);
	VOID    ScanScreenData(HDC  DestinationDCHandle, HDC SourceDCHandle, ULONG Width, ULONG Height);
	ULONG   CompareBitmapData(LPBYTE NextScreenData, LPBYTE FirstScreenData, LPBYTE BufferData, DWORD ScreenBufferLength);
private:
	BYTE   m_Algorithm;
	ULONG  m_BitmapCount;
	int    m_FullWidth;
	int    m_FullHeigth;
	LPBITMAPINFO  m_BitmapInfo;
	HWND     m_DesktopHwnd;
	HDC      m_DesktopDCHandle;
	HDC      m_DesktopMemoryDCHandle;
	HBITMAP  m_BitmapHandle;
	PVOID    m_BitmapData;
	BYTE*    m_BufferData;
	ULONG    m_Offset;
	HDC      m_DiffMemoryDCHandle;
	HBITMAP  m_DiffBitmapHandle;
	PVOID    m_DiffBitmapData;
};

