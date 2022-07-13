#include "ScreenSpy.h"


#define  ALGORITHM_DIFF  1
CScreenSpy::CScreenSpy()
{
}
CScreenSpy::CScreenSpy(ULONG  BitmapCount)
{
	m_Algorithm = ALGORITHM_DIFF;
	switch (BitmapCount)
	{
	case 16:
	case 32:
	{
		m_BitmapCount = BitmapCount;
		break;
	}
	default:
		m_BitmapCount = 16;
	}
	//获得屏幕分辨率
	m_FullHeigth = GetSystemMetrics(SM_CYSCREEN);
	m_FullWidth = GetSystemMetrics(SM_CXSCREEN);
	//构建位图信息
	m_BitmapInfo = OnInitBitmapInfo(m_BitmapCount, m_FullWidth, m_FullHeigth);
	//获得屏幕句柄
	m_DesktopHwnd = GetDesktopWindow();
	m_DesktopDCHandle = GetDC(m_DesktopHwnd);

	//第一帧
	m_DesktopMemoryDCHandle = CreateCompatibleDC(m_DesktopDCHandle);
	m_BitmapHandle = CreateDIBSection(m_DesktopDCHandle, m_BitmapInfo,
		DIB_RGB_COLORS, &m_BitmapData, NULL, NULL);
	SelectObject(m_DesktopMemoryDCHandle, m_BitmapHandle);

	m_DiffMemoryDCHandle = CreateCompatibleDC(m_DesktopDCHandle);
	m_DiffBitmapHandle = ::CreateDIBSection(m_DesktopDCHandle, m_BitmapInfo,
		DIB_RGB_COLORS, &m_DiffBitmapData, NULL, NULL);
	SelectObject(m_DiffMemoryDCHandle, m_DiffBitmapHandle);

	m_Offset = 0;
	m_BufferData = new BYTE[m_BitmapInfo->bmiHeader.biSizeImage * 2];
}

CScreenSpy::~CScreenSpy()
{
	ReleaseDC(m_DesktopHwnd, m_DesktopDCHandle);
	//回收工具箱
	if (m_DesktopMemoryDCHandle != NULL)
	{
		DeleteDC(m_DesktopMemoryDCHandle);
		DeleteObject(m_DiffBitmapHandle);
		if (m_BitmapData != NULL)
		{
			m_BitmapData = NULL;
		}
		m_DesktopMemoryDCHandle = NULL;
	}
	if (m_DiffMemoryDCHandle != NULL)
	{
		DeleteDC(m_DiffMemoryDCHandle);
		DeleteObject(m_BitmapHandle);
		if (m_DiffBitmapData != NULL)
		{
			m_BitmapData = NULL;
		}
		m_DiffMemoryDCHandle = NULL;
	}
	if (m_BitmapInfo != NULL)
	{
		delete[]  m_BitmapInfo;
		m_BitmapInfo = NULL;
	}
	if (m_BufferData)
	{
		delete[]  m_BufferData;
		m_BufferData = NULL;
	}
	m_Offset = 0;
}
LPBITMAPINFO  CScreenSpy::OnInitBitmapInfo(ULONG BitmapCount, ULONG FullWidth, ULONG FullHeigth)
{
	ULONG  BufferLength = sizeof(BITMAPINFOHEADER);
	BITMAPINFO*  BitmapInfo = (BITMAPINFO*)new  BYTE[BufferLength];
	BITMAPINFOHEADER*  BitmapInfoHeader = &(BitmapInfo->bmiHeader);
	BitmapInfoHeader->biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfoHeader->biWidth = FullWidth;
	BitmapInfoHeader->biHeight = FullHeigth;
	BitmapInfoHeader->biPlanes = 1;
	BitmapInfoHeader->biBitCount = BitmapCount;
	BitmapInfoHeader->biCompression = BI_RGB;
	BitmapInfoHeader->biXPelsPerMeter = 0;
	BitmapInfoHeader->biYPelsPerMeter = 0;
	BitmapInfoHeader->biClrImportant = 0;
	BitmapInfoHeader->biClrUsed = 0;
	BitmapInfoHeader->biSizeImage = ((BitmapInfoHeader->biWidth*BitmapInfoHeader->biBitCount + 31) / 32) * 4 * BitmapInfoHeader->biHeight;
	return BitmapInfo;
}
ULONG  CScreenSpy::GetBitmapInfoSize()
{
	return sizeof(BITMAPINFOHEADER);
}
LPBITMAPINFO  CScreenSpy::GetBitmapInfoData()
{
	return  m_BitmapInfo;
}
LPVOID  CScreenSpy::GetFirstScreenData()
{
	//用于从原设备中复制位图到目标设备
	::BitBlt(m_DesktopMemoryDCHandle, 0, 0,
		m_FullWidth, m_FullHeigth, m_DesktopDCHandle, 0, 0, SRCCOPY);
	return m_BitmapData;
}
ULONG  CScreenSpy::GetFirstScreenSize()
{
	return  m_BitmapInfo->bmiHeader.biSizeImage;
}
LPVOID  CScreenSpy::GetNextScreenData(ULONG* BufferLength)
{
	if (m_BufferData == NULL || BufferLength == NULL)
	{
		return  NULL;
	}
	m_Offset = 0;

	//写入使用了哪种算法
	WriteScreenData((LPBYTE)&m_Algorithm, sizeof(m_Algorithm));
	//m_BufferData = [m_Algorithm]

	//写入光标位置
	POINT  CursorPosition;
	GetCursorPos(&CursorPosition);
	WriteScreenData((LPBYTE)&CursorPosition, sizeof(POINT));
	//m_BufferData = [m_Algorithm][CursorPosition]

	//写入当前光标类型
	BYTE  CursorTypeIndex = -1;
	WriteScreenData(&CursorTypeIndex, sizeof(BYTE));
	//m_BufferData = [m_Algorithm][CursorPosition][CursorTypeIndex]

	//差异比较算法
	if (m_Algorithm == ALGORITHM_DIFF)
	{
		//分段扫描全屏幕，将新的位图放入到m_DiffMemoryDCHandle中
		ScanScreenData(m_DiffMemoryDCHandle, m_DesktopDCHandle, m_BitmapInfo->bmiHeader.biWidth, m_BitmapInfo->bmiHeader.biHeight);
		//两个位图进行比较，如果不一样修改
		*BufferLength = m_Offset +
			CompareBitmapData((LPBYTE)m_DiffBitmapData, (LPBYTE)m_BitmapData,
				m_BufferData + m_Offset, m_BitmapInfo->bmiHeader.biSizeImage);
		return  m_BufferData;
	}
	return  NULL;
}
void    CScreenSpy::WriteScreenData(LPBYTE  BufferData, ULONG  BufferLength)
{
	memcpy(m_BufferData + m_Offset, BufferData, BufferLength);
	m_Offset += BufferLength;
}
VOID    CScreenSpy::ScanScreenData(HDC  DestinationDCHandle, HDC SourceDCHandle, ULONG Width, ULONG Height)
{
	ULONG  JumpLine = 50;
	ULONG  JumpSleep = JumpLine / 10;
	for (int i = 0, ToJump = 0; i < Height; i += ToJump)
	{
		ULONG  v1 = Height - i;
		if (v1 > JumpLine)
		{
			ToJump = JumpLine;
		}
		else
		{
			ToJump = v1;
		}
		BitBlt(DestinationDCHandle, 0, i, Width, ToJump, SourceDCHandle, 0, i, SRCCOPY);
		Sleep(JumpSleep);
	}
}
ULONG   CScreenSpy::CompareBitmapData(LPBYTE NextScreenData, LPBYTE FirstScreenData, LPBYTE BufferData, DWORD ScreenBufferLength)
{
	//Windows规定一个扫描行所占的字节数必须是四的倍数，所以用DWORD比较
	LPDWORD  v1, v2;
	v1 = (LPDWORD)FirstScreenData;
	v2 = (LPDWORD)NextScreenData;
	ULONG  Offset = 0, v11 = 0, v22 = 0;
	ULONG  Count = 0;
	for (int i = 0; i < ScreenBufferLength; i += 4, v1++, v2++)
	{
		if (*v1 == *v2)
		{
			continue;
		}
		*(LPDWORD)(BufferData + Offset) = i;
		//记录数据大小的存放位置
		v11 = Offset + sizeof(DWORD);
		v22 = v11 + sizeof(DWORD);
		Count = 0;
		//更新前一帧数据
		*v1 = *v2;
		*(LPDWORD)(BufferData + v22 + Count) = *v2;
		Count += 4;
		i += 4, v1++, v2++;
		for (int j = i; j < ScreenBufferLength; j += 4, i += 4, v1++, v2++)
		{
			if (*v1 == *v2)
			{
				break;
			}
			*v1 = *v2;
			*(LPDWORD)(BufferData + v22 + Count) = *v2;
			Count += 4;

		}
		*(LPDWORD)(BufferData + v11) = Count;
		Offset = v22 + Count;
	}
	return Offset;	
}