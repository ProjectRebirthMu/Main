// ErrorReport.cpp: implementation of the CErrorReport class.
//
//////////////////////////////////////////////////////////////////////

#define DIRECTINPUT_VERSION 0x0800

#include "stdafx.h"
#include <ddraw.h>
#include <dinput.h>
#include <dmusicc.h>
#include <windows.h>
#include <eh.h>
#include <imagehlp.h>
#include "ErrorReport.h"

void DeleteSocket();

#define XOR_KEY_SIZE	(16)

void Xor_ConvertBlock( BYTE *lpBuffer, int iSize, int iKey)
{
	assert( iKey >= 0);
	assert( iSize >= 0);
	assert( iSize + iKey <= XOR_KEY_SIZE);

	BYTE byXorKey[XOR_KEY_SIZE] = { 0x7C, 0xBD, 0x81, 0x9F, 0x3D, 0x93, 0xE2, 0x56,
									0x2A, 0x73, 0xD2, 0x3E, 0xF2, 0x83, 0x95, 0xBF};

	for ( int i = 0; i < iSize; ++i)
	{
		lpBuffer[i] ^= byXorKey[i+iKey];
	}
}

int Xor_ConvertBuffer( void *lpBuffer, int iSize, int iKey = 0)
{
	int iSizeLeft = iSize;
	BYTE *lpCurrent = ( BYTE*)lpBuffer;

	int iConvertSize = min( ( XOR_KEY_SIZE - iKey) % XOR_KEY_SIZE, iSize);
	Xor_ConvertBlock( lpCurrent, iConvertSize, iKey);
	lpCurrent += iConvertSize;
	iSizeLeft -= iConvertSize;
	if ( iSizeLeft <= 0)
	{
		return ( iConvertSize + iKey);
	}

	while ( iSizeLeft >= XOR_KEY_SIZE)
	{
		iConvertSize = XOR_KEY_SIZE;
		Xor_ConvertBlock( lpCurrent, iConvertSize, 0);
		lpCurrent += iConvertSize;
		iSizeLeft -= iConvertSize;
	}

	iConvertSize = iSizeLeft;
	Xor_ConvertBlock( lpCurrent, iConvertSize, 0);
	return ( iConvertSize);
}

CErrorReport::CErrorReport()
{
	Clear();
	//Create("MuError.log");
}

CErrorReport::~CErrorReport()
{
	Destroy();
}

void CErrorReport::Clear( void)
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_lpszFileName[0] = '\0';
	m_iKey = 0;
}

void CErrorReport::Create(char* lpszFileName)
{
	strcpy_s(m_lpszFileName, MAX_PATH, lpszFileName); // Usa strcpy_s para garantir a segurança da cópia
	m_iKey = 0;
	m_hFile = CreateFile(m_lpszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_hFile == INVALID_HANDLE_VALUE) 
	{
	}

	CutHead();
	SetFilePointer(m_hFile, 0, NULL, FILE_END);
}

void CErrorReport::Destroy( void)
{
	CloseHandle( m_hFile);
	Clear();
}

void CErrorReport::CutHead()
{
	DWORD dwNumber = 0;
	char lpszBuffer[128 * 1024];
	// Read the file contents
	if (!ReadFile(m_hFile, lpszBuffer, 128 * 1024 - 1, &dwNumber, NULL))
	{
		// Error handling
		return;
	}
	// Null-terminate the buffer
	lpszBuffer[dwNumber] = '\0';
	// Find the cut point
	char* lpCut = CheckHeadToCut(lpszBuffer, dwNumber);
	if (dwNumber >= 32 * 1024 - 1)
	{
		lpCut = &lpszBuffer[32 * 1024 - 1];
	}
	// Cut the head of the file
	if (lpCut != lpszBuffer)
	{
		CloseHandle(m_hFile);
		DeleteFile(m_lpszFileName);
		m_hFile = CreateFile(m_lpszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD dwSize = dwNumber - (lpCut - lpszBuffer);
		if (dwSize > 0)
		{
			// Write the remaining content to the file
			if (!WriteFile(m_hFile, lpCut, dwSize, &dwNumber, NULL))
			{
				// Error handling
				return;
			}
		}
	}
}

char* CErrorReport::CheckHeadToCut( char *lpszBuffer, DWORD dwNumber)
{
	char *lpszBegin = "###### Log Begin ######";
	int iLengthOfBegin = strlen( lpszBegin);

	char *lpFoundList[128];
	int iFoundCount = 0;

	for ( char *lpFind = lpszBuffer; lpFind && *lpFind; )
	{
		lpFind = strchr( lpFind, ( int)'#');
		if ( lpFind)
		{
			if ( 0 == strncmp( lpFind, lpszBegin, iLengthOfBegin))
			{
				lpFoundList[iFoundCount++] = lpFind;
				lpFind += iLengthOfBegin;
			}
			else
			{
				lpFind++;
			}
		}
	}

	if ( iFoundCount >= 5)
	{
		return ( lpFoundList[iFoundCount-4]);
	}
	return ( lpszBuffer);
}

BOOL CErrorReport::WriteFile(HANDLE hFile, void* lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	return ::WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}


void CErrorReport::WriteDebugInfoStr( char *lpszToWrite)
{
	if ( m_hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwNumber;
		WriteFile( m_hFile, lpszToWrite, strlen( lpszToWrite), &dwNumber, NULL);

		if ( dwNumber == 0)
		{
			CloseHandle( m_hFile);
			Create( m_lpszFileName);
		}
	}
}

void CErrorReport::Write( const char* lpszFormat, ...)
{
	char lpszBuffer[1024] = {0, };
	va_list va;
	va_start( va, lpszFormat);
	vsprintf( lpszBuffer, lpszFormat, va);
	va_end( va);

	WriteDebugInfoStr( lpszBuffer);
}

void CErrorReport::HexWrite(void* pBuffer, int iSize)
{
	DWORD dwWritten = 0;
	char szLine[256] = { 0 };
	int offset = 0;
	DWORD_PTR ptr = reinterpret_cast<DWORD_PTR>(pBuffer);
	offset += sprintf(szLine, "0x%08X:", ptr);
	for (int i = 0; i < iSize; i++) {
		offset += sprintf(szLine + offset, "%02X", ((BYTE)pBuffer + i));
		if (i > 0 && i < iSize - 1) {
			if (i % 16 == 15) { //. new line
				offset += sprintf(szLine + offset, "\r\n");
				WriteFile(m_hFile, szLine, static_cast<DWORD>(strlen(szLine)), &dwWritten, NULL);
				offset = 0;
				ptr += 16;
				offset += sprintf(szLine + offset, "0x%08X: ", ptr);
			}
			else if (i % 4 == 3) { //. space
				offset += sprintf(szLine + offset, " ");
			}
		}
	}
	offset += sprintf(szLine + offset, "\r\n");
	WriteFile(m_hFile, szLine, static_cast<DWORD>(strlen(szLine)), &dwWritten, NULL);
}

void CErrorReport::AddSeparator( void)
{
	Write( "-------------------------------------------------------------------------------------\r\n");
}

void CErrorReport::WriteLogBegin( void)
{
	Write( "###### Log Begin ######\r\n");
}

void CErrorReport::WriteCurrentTime( BOOL bLineShift)
{
	SYSTEMTIME st;
	GetLocalTime( &st);
	g_ErrorReport.Write( "%4d/%02d/%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	if ( bLineShift)
	{
		g_ErrorReport.Write( "\r\n");
	}
}

void CErrorReport::WriteSystemInfo(ER_SystemInfo* si)
{
	Write("<System information>\r\n");
	Write("OS\t\t\t: %s\r\n", si->m_lpszOS);
	Write("CPU\t\t\t: %s\r\n", si->m_lpszCPU);
	Write("RAM\t\t\t: %dMB\r\n", 1 + (si->m_iMemorySize / 1024 / 1024));
	AddSeparator();
	Write("Direct-X\t\t: %s\r\n", si->m_lpszDxVersion);
}

void CErrorReport::WriteOpenGLInfo()
{
	const char* vendor = (const char*)glGetString(GL_VENDOR);
	const char* renderer = (const char*)glGetString(GL_RENDERER);
	const char* version = (const char*)glGetString(GL_VERSION);
	Write("<OpenGL information>\r\n");
	Write("Vendor\t\t: %s\r\n", vendor ? vendor : "unknown");
	Write("Renderer\t: %s\r\n", renderer ? renderer : "unknown");
	Write("OpenGL version\t: %s\r\n", version ? version : "unknown");

	GLint maxTextureSize[2] = { 0 };
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxTextureSize);
	Write("Max Texture size\t: %d x %d\r\n", maxTextureSize[0], maxTextureSize[1]);

	GLint maxViewportDims[2] = { 0 };
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);
	Write("Max Viewport size\t: %d x %d\r\n", maxViewportDims[0], maxViewportDims[1]);
}

void CErrorReport::WriteImeInfo( HWND hWnd)
{
	char lpszTemp[256];
	Write( "<IME information>\r\n");

	HIMC hImc = ImmGetContext( hWnd);
	if ( hImc)
	{
		HKL hKl = GetKeyboardLayout( 0);
		ImmGetDescription( hKl, lpszTemp, 256);
		Write( "IME Name\t\t: %s\r\n", lpszTemp);
		ImmGetIMEFileName( hKl, lpszTemp, 256);
		Write( "IME File Name\t\t: %s\r\n", lpszTemp);
		ImmReleaseContext( hWnd, hImc);
	}
	GetKeyboardLayoutName( lpszTemp);
	Write( "Keyboard type\t\t: %s\r\n", lpszTemp);
}

typedef struct tagER_SOUNDDEVICE{
	char	szGuid[64];
	char	szDeviceName[128];
	char	szDriverName[128];
} ER_SOUNDDEVICEINFO;

typedef struct tagSOUNDDEVICEENUM{
	enum { MAX_DEVICENUM = 20 };
	tagSOUNDDEVICEENUM(){ nDeivceCount = 0; }
	ER_SOUNDDEVICEINFO		infoSoundDevice[MAX_DEVICENUM];
	size_t				nDeivceCount;

	tagER_SOUNDDEVICE& operator [] (size_t p){
		return infoSoundDevice[p];
	}
	tagER_SOUNDDEVICE& GetNextDevice(){ 
		return infoSoundDevice[nDeivceCount];
	}
} ER_SOUNDDEVICEENUMINFO;

INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, LPSTR strDesc, LPSTR strDrvName, VOID* pContext )
{
	if( pGUID ){
		ER_SOUNDDEVICEENUMINFO* pSoundDeviceEnumInfo = (ER_SOUNDDEVICEENUMINFO*)pContext;
		strcpy(pSoundDeviceEnumInfo->GetNextDevice().szDeviceName, strDesc);
		strcpy(pSoundDeviceEnumInfo->GetNextDevice().szDriverName, strDrvName);
		pSoundDeviceEnumInfo->nDeivceCount++;
	}
	return TRUE;
}

BOOL GetFileVersion( char *lpszFileName, WORD *pwVersion);

void CErrorReport::WriteSoundCardInfo( void)
{
	ER_SOUNDDEVICEENUMINFO sdi;
	DirectSoundEnumerate( (LPDSENUMCALLBACK)DSoundEnumCallback, &sdi);

	if ( sdi.nDeivceCount > 0)
	{
		Write( "<Sound card information>\r\n");
	}
	else
	{
		Write( "No sound card found.\r\n");
		return;
	}

	for ( unsigned int i = 0; i < sdi.nDeivceCount; ++i)
	{
		Write( "Sound Card \t\t: %s\r\n", sdi.infoSoundDevice[i].szDeviceName);

		char lpszBuffer[MAX_PATH];
		GetSystemDirectory( lpszBuffer, MAX_PATH);
		strcat( lpszBuffer, "\\drivers\\");
		strcat( lpszBuffer, sdi.infoSoundDevice[i].szDriverName);
		WORD wVersion[4];
		GetFileVersion( lpszBuffer, wVersion);

		Write( "Sound Card Driver\t: %s (%d.%d.%d.%d)\r\n", sdi.infoSoundDevice[i].szDriverName, wVersion[0], wVersion[1], wVersion[2], wVersion[3]);
	}

	AddSeparator();
}

void GetOSVersion(ER_SystemInfo* si) 
{
	char* lpszUnknown = "Unknown";
	char lpszTemp[256];

	OSVERSIONINFOEX osi;
	ZeroMemory(&osi, sizeof(OSVERSIONINFOEX));
	osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO*)&osi);

	int iBuildNumberType = 0;
	wsprintf( si->m_lpszOS, "%s %d.%d ", lpszUnknown, osi.dwMajorVersion, osi.dwMinorVersion);

	switch (osi.dwMajorVersion)
	{
	case 11:
	{
		if (osi.dwMinorVersion == 0) {
			strcpy(si->m_lpszOS, "Windows 11");
			if (osi.wProductType == VER_NT_WORKSTATION) {
				strcat(si->m_lpszOS, " Home");
			}
			else if (osi.wProductType == VER_NT_SERVER) {
				strcat(si->m_lpszOS, " Server");
			}
			else if (osi.wProductType == VER_NT_DOMAIN_CONTROLLER) {
				strcat(si->m_lpszOS, " Domain Controller");
			}
		}
		break;
	}
	case 10:
	{
		if (osi.dwMinorVersion == 0)
		{
			strcpy(si->m_lpszOS, "Windows 10");
			if (osi.wProductType == VER_NT_WORKSTATION)
			{
				strcat(si->m_lpszOS, " Home");
			}
			else if (osi.wProductType == VER_NT_SERVER)
			{
				strcat(si->m_lpszOS, " Server");
			}
			else if (osi.wProductType == VER_NT_DOMAIN_CONTROLLER)
			{
				strcat(si->m_lpszOS, " Domain Controller");
			}
		}
		else if (osi.dwMinorVersion == 1) {
			strcpy(si->m_lpszOS, "Windows 10, version 1511");
		}
		else if (osi.dwMinorVersion == 2) {
			strcpy(si->m_lpszOS, "Windows 10, version 1607");
		}
		else if (osi.dwMinorVersion == 3) {
			strcpy(si->m_lpszOS, "Windows 10, version 1703");
		}
		else if (osi.dwMinorVersion == 4) {
			strcpy(si->m_lpszOS, "Windows 10, version 1709");
		}
		else if (osi.dwMinorVersion == 5) {
			strcpy(si->m_lpszOS, "Windows 10, version 1803");
		}
		else if (osi.dwMinorVersion == 6) {
			strcpy(si->m_lpszOS, "Windows 10, version 1809");
		}
		else if (osi.dwMinorVersion == 7) {
			strcpy(si->m_lpszOS, "Windows 10, version 1903");
		}
		else if (osi.dwMinorVersion == 8) {
			strcpy(si->m_lpszOS, "Windows 10, version 1909");
		}
		else if (osi.dwMinorVersion == 9) {
			strcpy(si->m_lpszOS, "Windows 10, version 2004");
		}
		else if (osi.dwMinorVersion == 10) {
			strcpy(si->m_lpszOS, "Windows 10, version 20H2");
		}
		else if (osi.dwMinorVersion == 11) {
			strcpy(si->m_lpszOS, "Windows 10, version 21H1");
		}
		break;
	}
	case 6: {
		if (osi.dwMinorVersion == 3) {
			if (osi.wProductType == VER_NT_WORKSTATION) {
				strcpy(si->m_lpszOS, "Windows 8.1");
			}
			else {
				strcpy(si->m_lpszOS, "Windows Server 2012 R2");
			}
		}
		else if (osi.dwMinorVersion == 2) {
			if (osi.wProductType == VER_NT_WORKSTATION) {
				strcpy(si->m_lpszOS, "Windows 8");
			}
			else {
				strcpy(si->m_lpszOS, "Windows Server 2012");
			}
		}
		else if (osi.dwMinorVersion == 1) {
			if (osi.wProductType == VER_NT_WORKSTATION) {
				strcpy(si->m_lpszOS, "Windows 7");
			}
			else {
				strcpy(si->m_lpszOS, "Windows Server 2008 R2");
			}
		}
		else if (osi.dwMinorVersion == 0) {
			if (osi.wProductType == VER_NT_WORKSTATION) {
				strcpy(si->m_lpszOS, "Windows Vista");
			}
			else {
				strcpy(si->m_lpszOS, "Windows Server 2008");
			}
		}
		break;
	}
		  switch (iBuildNumberType)
		  {
		  case 0:
			  wsprintf(lpszTemp, "Build %d ", osi.dwBuildNumber);
			  break;
		  case 1:
			  wsprintf(lpszTemp, "Build %d.%d.%d ", HIBYTE(HIWORD(osi.dwBuildNumber)), LOBYTE(HIWORD(osi.dwBuildNumber)), LOWORD(osi.dwBuildNumber));
			  break;
		  }
		  strcat(si->m_lpszOS, lpszTemp);
		  wsprintf(lpszTemp, "(%s)", osi.szCSDVersion);
		  strcat(si->m_lpszOS, lpszTemp);
	}
}

int64_t GetCPUFrequency(unsigned int uiMeasureMSecs)
{
	assert(uiMeasureMSecs > 0);

	// First we get the CPUID standard level 0x00000001
	uint32_t reg;

	__asm
	{
		mov eax, 1
		cpuid
		mov reg, edx
	}

	// Then we check if the RDTSC (Real Date Time Stamp Counter) is available.
	// This function is necessary for our measure process.
	if (!(reg & (1 << 4)))
	{
		return 0;
	}

	// After that, we declare some variables and check the frequency of the high
	// resolution timer for the measure process.
	// If there's no high-res timer, we exit.
	int64_t llFreq;
	if (!QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&llFreq)))
	{
		return 0;
	}

	// Now we can init the measure process. We set the process and thread priority
	// to the highest available level (Realtime priority). Also, we focus on the
	// first processor in the multiprocessor system.
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	DWORD dwCurPriorityClass = GetPriorityClass(hProcess);
	int iCurThreadPriority = GetThreadPriority(hThread);
	DWORD_PTR dwProcessMask, dwSystemMask, dwNewMask = 1;
	GetProcessAffinityMask(hProcess, &dwProcessMask, &dwSystemMask);

	SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
	SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
	SetProcessAffinityMask(hProcess, dwNewMask);

	// Now we call a CPUID to ensure that all other prior called functions are
	// completed now (serialization)
	__asm { cpuid }

	int64_t llStartTime, llEndTime;
	int64_t llStart, llEnd;
	// We ask the high-res timer for the start time
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&llStartTime));
	// Then we get the current CPU clock and store it
	__asm
	{
		rdtsc
		mov dword ptr[llStart + 4], edx
		mov dword ptr[llStart], eax
	}

	// Now we wait for some milliseconds
	Sleep(uiMeasureMSecs);

	// We ask for the end time
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&llEndTime));
	// And also for the end CPU clock
	__asm
	{
		rdtsc
		mov dword ptr[llEnd + 4], edx
		mov dword ptr[llEnd], eax
	}

	// Now we can restore the default process and thread priorities
	SetProcessAffinityMask(hProcess, dwProcessMask);
	SetThreadPriority(hThread, iCurThreadPriority);
	SetPriorityClass(hProcess, dwCurPriorityClass);

	// Then we calculate the time and clock differences
	int64_t llDif = llEnd - llStart;
	int64_t llTimeDif = llEndTime - llStartTime;

	// And finally the frequency is the clock difference divided by the time
	// difference.
	int64_t llFrequency = static_cast<int64_t>(static_cast<double>(llDif) / (static_cast<double>(llTimeDif) / llFreq));

	// At last, we just return the frequency that is also stored in the call
	// member var uqwFrequency
	return llFrequency;
}

void GetCPUInfo( ER_SystemInfo *si)
{
	char *lpszUnknown = "Unknown";
	char lpszTemp[256];

	DWORD eaxreg, ebxreg, ecxreg, edxreg;
 
	// We read the standard CPUID level 0x00000000 which should
	// be available on every x86 processor
	__asm
	{
		mov eax, 0
		cpuid
		mov eaxreg, eax
		mov ebxreg, ebx
		mov edxreg, edx
		mov ecxreg, ecx
	}

	int iBrand = ebxreg;
	*((DWORD *) si->m_lpszCPU) = ebxreg;
	*((DWORD *) (si->m_lpszCPU+4)) = edxreg;
	*((DWORD *) (si->m_lpszCPU+8)) = ecxreg;
	strcat( si->m_lpszCPU, " - ");
	unsigned long ulMaxSupportedLevel, ulMaxSupportedExtendedLevel;
	ulMaxSupportedLevel = eaxreg & 0xFFFF;
	// Then we read the ext. CPUID level 0x80000000
	// ...to check the max. supportted extended CPUID level

	__asm
	{
        mov eax, 0x80000000
		cpuid
		mov eaxreg, eax
	}
	ulMaxSupportedExtendedLevel = eaxreg;

	// First we get the CPUID standard level 0x00000001
	__asm
	{
		mov eax, 1
		cpuid
		mov eaxreg, eax
	}

	unsigned int uiFamily   = (eaxreg >> 8) & 0xF;
	unsigned int uiModel    = (eaxreg >> 4) & 0xF;

	switch ( iBrand)
	{
		case 0x756E6547:	// GenuineIntel
			switch ( uiFamily)
			{
			case 3:	// 386
				break;
			case 4:	// 486
				break;
			case 5:	// pentium
				break;
			case 6:	// pentium pro - pentium 3
				switch ( uiModel)
				{
				case 0:
				case 1:
				default:
					strcat( si->m_lpszCPU, "Pentium Pro");
					break;
				case 3:
				case 5:
					strcat( si->m_lpszCPU, "Pentium 2");
					break;
				case 6:
					strcat( si->m_lpszCPU, "Pentium Celeron");
					break;
				case 7:
				case 8:
				case 0xA:
				case 0xB:
					strcat( si->m_lpszCPU, "Pentium 3");
					break;
				}
				break;
			case 15:	// pentium 4
				strcat( si->m_lpszCPU, "Pentium 4");
				break;
			default:
				strcat( si->m_lpszCPU, lpszUnknown);
				break;
			}
			break;
		case 0x68747541:	// AuthenticAMD
			switch ( uiFamily)
			{
			case 4:	// 486, 586
				switch ( uiModel)
				{
				case 3:
				case 7:
				case 8:
				case 9:
					strcat( si->m_lpszCPU, "AMD 486");
					break;
				case 0xE:
				case 0xF:
					strcat( si->m_lpszCPU, "AMD 586");
					break;
				default:
					strcat( si->m_lpszCPU, "AMD Unknown (486 or 586)");
					break;
				}
			case 5:	// K5, K6
				switch ( uiModel)
				{
				case 0:
				case 1:
				case 2:
				case 3:
					strcat( si->m_lpszCPU, "AMD K5 5k86");
					break;
				case 6:
				case 7:
					strcat( si->m_lpszCPU, "AMD K6");
					break;
				case 8:
					strcat( si->m_lpszCPU, "AMD K6-2");
					break;
				case 9:
					strcat( si->m_lpszCPU, "AMD K6-3");
					break;
				case 0xD:
					strcat( si->m_lpszCPU, "AMD K6-2+ or K6-3+");
					break;
				default:
					strcat( si->m_lpszCPU, "AMD Unknown (K5 or K6)");
					break;
				}
				break;
			case 6:	// K7 Athlon, Duron
				switch ( uiModel)
				{
				case 1:
				case 2:
				case 4:
				case 6:
					strcat( si->m_lpszCPU, "AMD K-7 Athlon");
					break;
				case 3:
				case 7:
					strcat( si->m_lpszCPU, "AMD K-7 Duron");
					break;
				default:
					strcat( si->m_lpszCPU, "AMD K-7 Unknown");
					break;
				}
				break;
			default:
				strcat( si->m_lpszCPU, "AMD Unknown");
				break;
			}
			break;
		case 0x69727943:	// CyrixInstead
		default:
			strcat( si->m_lpszCPU, lpszUnknown);
			break;
	}

	__int64 llFreq = GetCPUFrequency( 50) / 1000000;
	wsprintf( lpszTemp, " %dMhz", ( int)llFreq);
	strcat( si->m_lpszCPU, lpszTemp);
}

typedef HRESULT(WINAPI* LPDIRECTDRAWCREATE)(GUID*, LPDIRECTDRAW*, IUnknown*);
typedef HRESULT(WINAPI* LPDIRECTDRAWCREATEEX)(GUID*, VOID**, REFIID, LPUNKNOWN);
typedef HRESULT(WINAPI* LPDIRECTINPUTCREATE)(HINSTANCE, DWORD, LPDIRECTINPUT*, LPUNKNOWN);

DWORD GetDXVersion()
{
    LPDIRECTDRAWCREATE   DirectDrawCreate   = NULL;
	LPDIRECTDRAWCREATEEX DirectDrawCreateEx = NULL;
	LPDIRECTINPUTCREATE  DirectInputCreate  = NULL;
    HINSTANCE            hDDrawDLL          = NULL;
    HINSTANCE            hDInputDLL         = NULL;
    HINSTANCE            hD3D8DLL           = NULL;
	HINSTANCE			 hD3D9DLL			= NULL;
    LPDIRECTDRAW         pDDraw             = NULL;
    LPDIRECTDRAW2        pDDraw2            = NULL;
    LPDIRECTDRAWSURFACE  pSurf              = NULL;
    LPDIRECTDRAWSURFACE3 pSurf3             = NULL;
    LPDIRECTDRAWSURFACE4 pSurf4             = NULL;
    DWORD                dwDXVersion        = 0;
    HRESULT              hr;

    // First see if DDRAW.DLL even exists.
    hDDrawDLL = LoadLibrary( "DDRAW.DLL" );
    if( hDDrawDLL == NULL )
    {
        dwDXVersion = 0;
        return dwDXVersion;
    }

    // See if we can create the DirectDraw object.
    DirectDrawCreate = (LPDIRECTDRAWCREATE)GetProcAddress( hDDrawDLL, "DirectDrawCreate" );
    if( DirectDrawCreate == NULL )
    {
        dwDXVersion = 0;
        FreeLibrary( hDDrawDLL );

		__TraceF(TEXT("===> Couldn't LoadLibrary DDraw\r\n"));
		return dwDXVersion;
    }

    hr = DirectDrawCreate( NULL, &pDDraw, NULL );
    if( FAILED(hr) )
    {
        dwDXVersion = 0;
        FreeLibrary( hDDrawDLL );
		__TraceF(TEXT("===> Couldn't create DDraw\r\n"));
		return dwDXVersion;
    }

    // So DirectDraw exists.  We are at least DX1.
    dwDXVersion = 0x100;

    // Let's see if IID_IDirectDraw2 exists.
    hr = pDDraw->QueryInterface( IID_IDirectDraw2, (VOID**)&pDDraw2 );
    if( FAILED(hr) )
    {
        // No IDirectDraw2 exists... must be DX1
        pDDraw->Release();
        FreeLibrary( hDDrawDLL );
		__TraceF(TEXT("===> Couldn't QI DDraw2\r\n"));
        return dwDXVersion;
    }

    // IDirectDraw2 exists. We must be at least DX2
    pDDraw2->Release();
    dwDXVersion = 0x200;


	//-------------------------------------------------------------------------
    // DirectX 3.0 Checks
	//-------------------------------------------------------------------------

    // DirectInput was added for DX3
    hDInputDLL = LoadLibrary( "DINPUT.DLL" );
    if( hDInputDLL == NULL )
    {
        // No DInput... must not be DX3
		__TraceF(TEXT("===> Couldn't LoadLibrary DInput\r\n"));
        pDDraw->Release();
        return dwDXVersion;
    }

    DirectInputCreate = (LPDIRECTINPUTCREATE)GetProcAddress( hDInputDLL,
                                                        "DirectInputCreateA" );
    if( DirectInputCreate == NULL )
    {
        // No DInput... must be DX2
        FreeLibrary( hDInputDLL );
        FreeLibrary( hDDrawDLL );
        pDDraw->Release();
		__TraceF(TEXT("===> Couldn't GetProcAddress DInputCreate\r\n"));
        return dwDXVersion;
    }

    // DirectInputCreate exists. We are at least DX3
    dwDXVersion = 0x300;
    FreeLibrary( hDInputDLL );

    // Can do checks for 3a vs 3b here


	//-------------------------------------------------------------------------
    // DirectX 5.0 Checks
	//-------------------------------------------------------------------------

    // We can tell if DX5 is present by checking for the existence of
    // IDirectDrawSurface3. First, we need a surface to QI off of.
    DDSURFACEDESC ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    hr = pDDraw->SetCooperativeLevel( NULL, DDSCL_NORMAL );
    if( FAILED(hr) )
    {
        // Failure. This means DDraw isn't properly installed.
        pDDraw->Release();
        FreeLibrary( hDDrawDLL );
        dwDXVersion = 0;
		__TraceF(TEXT("===> Couldn't Set coop level\r\n"));
        return dwDXVersion;
    }

    hr = pDDraw->CreateSurface( &ddsd, &pSurf, NULL );
    if( FAILED(hr) )
    {
        // Failure. This means DDraw isn't properly installed.
        pDDraw->Release();
        FreeLibrary( hDDrawDLL );
        dwDXVersion = 0;
		__TraceF(TEXT("===> Couldn't CreateSurface\r\n"));
        return dwDXVersion;
    }

    // Query for the IDirectDrawSurface3 interface
    if( FAILED( pSurf->QueryInterface( IID_IDirectDrawSurface3,
                                       (VOID**)&pSurf3 ) ) )
    {
        pDDraw->Release();
        FreeLibrary( hDDrawDLL );
        return dwDXVersion;
    }

    // QI for IDirectDrawSurface3 succeeded. We must be at least DX5
    dwDXVersion = 0x500;


	//-------------------------------------------------------------------------
    // DirectX 6.0 Checks
	//-------------------------------------------------------------------------

    // The IDirectDrawSurface4 interface was introduced with DX 6.0
    if( FAILED( pSurf->QueryInterface( IID_IDirectDrawSurface4,
                                       (VOID**)&pSurf4 ) ) )
    {
        pDDraw->Release();
        FreeLibrary( hDDrawDLL );
        return dwDXVersion;
    }

    // IDirectDrawSurface4 was create successfully. We must be at least DX6
    dwDXVersion = 0x600;
    pSurf->Release();
    pDDraw->Release();


	//-------------------------------------------------------------------------
    // DirectX 6.1 Checks
	//-------------------------------------------------------------------------

    // Check for DMusic, which was introduced with DX6.1
    LPDIRECTMUSIC pDMusic = NULL;
    CoInitialize( NULL );
    hr = CoCreateInstance( CLSID_DirectMusic, NULL, CLSCTX_INPROC_SERVER,
                           IID_IDirectMusic, (VOID**)&pDMusic );
    if( FAILED(hr) )
    {
		__TraceF(TEXT("===> Couldn't create CLSID_DirectMusic\r\n"));
        FreeLibrary( hDDrawDLL );
        return dwDXVersion;
    }

    // DirectMusic was created successfully. We must be at least DX6.1
    dwDXVersion = 0x601;
    pDMusic->Release();
    CoUninitialize();
    

	//-------------------------------------------------------------------------
    // DirectX 7.0 Checks
	//-------------------------------------------------------------------------

    // Check for DirectX 7 by creating a DDraw7 object
    LPDIRECTDRAW7 pDD7;
    DirectDrawCreateEx = (LPDIRECTDRAWCREATEEX)GetProcAddress( hDDrawDLL,
                                                       "DirectDrawCreateEx" );
    if( NULL == DirectDrawCreateEx )
    {
        FreeLibrary( hDDrawDLL );
        return dwDXVersion;
    }

    if( FAILED( DirectDrawCreateEx( NULL, (VOID**)&pDD7, IID_IDirectDraw7,
                                    NULL ) ) )
    {
        FreeLibrary( hDDrawDLL );
        return dwDXVersion;
    }

    // DDraw7 was created successfully. We must be at least DX7.0
    dwDXVersion = 0x700;
    pDD7->Release();


	//-------------------------------------------------------------------------
    // DirectX 8.0 Checks
	//-------------------------------------------------------------------------

    // Simply see if D3D8.dll exists.
    hD3D8DLL = LoadLibrary( "D3D8.DLL" );
    if( hD3D8DLL == NULL )
    {
	    FreeLibrary( hDDrawDLL );
        return dwDXVersion;
    }

    // D3D8.dll exists. We must be at least DX8.0
    dwDXVersion = 0x800;

	
	//-------------------------------------------------------------------------
	// DirectX 9.0 Checks
	//-------------------------------------------------------------------------
	hD3D9DLL = LoadLibrary( "D3D9.DLL" );
	if( hD3D9DLL == NULL )
	{
		FreeLibrary( hDDrawDLL );
		FreeLibrary( hD3D8DLL );
		return dwDXVersion;
	}
	dwDXVersion = 0x900;

	//-------------------------------------------------------------------------
    // End of checking for versions of DirectX 
	//-------------------------------------------------------------------------

    // Close open libraries and return
    FreeLibrary( hDDrawDLL );
    FreeLibrary( hD3D8DLL );
	FreeLibrary( hD3D9DLL );
    
    return dwDXVersion;
}

void GetSystemInfo(ER_SystemInfo* si) {
	// Clear memory
	ZeroMemory(si, sizeof(ER_SystemInfo));
	// Get CPU information
	GetCPUInfo(si);

	// Get system memory information
	MEMORYSTATUSEX ms;
	ms.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&ms);
	si->m_iMemorySize = ms.ullTotalPhys;

	// Get operating system information
	GetOSVersion(si);

	// Get Direct X version information
	DWORD dwDX = GetDXVersion();
	sprintf_s(si->m_lpszDxVersion, sizeof(si->m_lpszDxVersion), "DirectX %d.%d", dwDX >> 8, dwDX & 0xFF);
}

