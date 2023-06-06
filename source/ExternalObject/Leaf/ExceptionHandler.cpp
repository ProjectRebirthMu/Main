#include "stdafx.h"

#include "ExceptionHandler.h"
#include "checkintegrity.h"

#include <tlhelp32.h>
#include <time.h>

//. Wrapping to C-style function
bool leaf::AttachExceptionHandler(const std::string& dmpfile, bool bSecondChance) { 
	return CExceptionHandler::GetObjPtr()->AttachExceptionHandler(dmpfile, bSecondChance); 
}
bool leaf::AttachExceptionHandler(EXCEPTION_CALLBACK pfCallback, bool bSecondChance) {
	return CExceptionHandler::GetObjPtr()->AttachExceptionHandler(pfCallback, bSecondChance);
}
bool leaf::DetachExceptionHandler() { 
	return CExceptionHandler::GetObjPtr()->DetachExceptionHandler(); 
}
bool leaf::IsContinueExceptionToSecondChance() {
	return CExceptionHandler::GetObjPtr()->IsEnableSecondChance();
}
bool leaf::SaveExceptionDumpFile(const std::string& filename, CONTEXT* pContext, _EXCEPTION_POINTERS* pExceptionInfo) {
	return CExceptionHandler::GetObjPtr()->SaveDmpFile(filename, pContext, pExceptionInfo);
}


//. CExceptionHandler

using namespace leaf;

CExceptionHandler::CExceptionHandler() : m_pfPrevExceptionFilter(NULL), m_pfExceptionCallback(NULL), m_bSecondChance(false) 
{}
CExceptionHandler::~CExceptionHandler() 
{}

bool CExceptionHandler::AttachExceptionHandler(const std::string& filename, bool bSecondChance)
{
	if(!m_pfPrevExceptionFilter) {
		m_pfPrevExceptionFilter = ::SetUnhandledExceptionFilter(CExceptionHandler::OnException);
		m_filename = filename;
		m_bSecondChance = bSecondChance;
		return true;
	}
	return false;
}
bool CExceptionHandler::AttachExceptionHandler(EXCEPTION_CALLBACK pfCallback, bool bSecondChance)
{
	if(!m_pfPrevExceptionFilter) {
		m_pfPrevExceptionFilter = ::SetUnhandledExceptionFilter(CExceptionHandler::OnException);
		m_pfExceptionCallback = pfCallback;
		m_bSecondChance = bSecondChance;
		return true;
	}
	return false;
}
bool CExceptionHandler::DetachExceptionHandler() 
{
	if(m_pfPrevExceptionFilter) {
		::SetUnhandledExceptionFilter(m_pfPrevExceptionFilter);
		m_pfPrevExceptionFilter = NULL;
		m_filename = "";
		m_bSecondChance = false;
		return true;
	}
	return false;
}
bool CExceptionHandler::IsEnableSecondChance() const 
{ return m_bSecondChance; }

const std::string& CExceptionHandler::GetDmpFileName() const
{ return m_filename; }
bool CExceptionHandler::SaveDmpFile(const std::string& filename, CONTEXT* pContext, _EXCEPTION_POINTERS* pExceptionInfo)
{
	if(pContext == NULL)
		return false;
	
	FILE* fd = fopen(filename.c_str(), "wb");
	if(fd == NULL) return false;
	
	DMPFILEHEADER DmpHeader;
	ZeroMemory(&DmpHeader, sizeof(DMPFILEHEADER));
	
	DmpHeader.Sign[0] = 'W';
	DmpHeader.Sign[1] = 'D';
	DmpHeader.Sign[2] = 'M';
	DmpHeader.Sign[3] = 'P';
	DmpHeader.Version = 0x03;
	DmpHeader.HeaderSize = sizeof(DMPFILEHEADER);
	
	//. Creation information
	DmpHeader.CreationTimeStamp = time(nullptr);
	
	//. System information
	OSVERSIONINFOEX osInfo;
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFOEX));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((LPOSVERSIONINFO)&osInfo);

	char osInfoStr[128];
	strncpy_s(osInfoStr, 128, (const char*)osInfo.szCSDVersion, 127);
	strncat_s(osInfoStr, 128, " ", 1);
	strncat_s(osInfoStr, 128, std::to_string(osInfo.dwMajorVersion).c_str(), 127);
	strncat_s(osInfoStr, 128, ".", 1);
	strncat_s(osInfoStr, 128, std::to_string(osInfo.dwMinorVersion).c_str(), 127);
	strncpy_s(DmpHeader.OSInfoString, 128, osInfoStr, 127);

	//. CPU information
	SYSTEM_INFO sysInfo;
	ZeroMemory(&sysInfo, sizeof(SYSTEM_INFO));
	GetSystemInfo(&sysInfo);

	char cpuInfoStr[128];
	strncpy_s(cpuInfoStr, 128, "Processor architecture: ", 127);
	strncat_s(cpuInfoStr, 128, std::to_string(sysInfo.wProcessorArchitecture).c_str(), 127);
	strncat_s(cpuInfoStr, 128, ", Number of processors: ", 127);
	strncat_s(cpuInfoStr, 128, std::to_string(sysInfo.dwNumberOfProcessors).c_str(), 127);
	strncpy_s(DmpHeader.CPUInfoString, 128, cpuInfoStr, 127);

	//. Ram Status
	MEMORYSTATUSEX memStatus;
	memStatus.dwLength = sizeof(memStatus);
	if (GlobalMemoryStatusEx(&memStatus)) {
		DmpHeader.MemStatus.dwLength = sizeof(MEMORYSTATUS);
		DmpHeader.MemStatus.dwTotalPhys = memStatus.ullTotalPhys;
		DmpHeader.MemStatus.dwAvailPhys = memStatus.ullAvailPhys;
	}
	else {
		// Em caso de erro...
	}

	//. Process header information
	std::vector<DMPMODULEINFO> ModuleInfo(64);
	ModuleInfo.assign(64, DMPMODULEINFO());

	DMPMODULEINFO* pModuleInfo = ModuleInfo.data();
	VS_FIXEDFILEINFO ImageFileInfo = { 0 };
	SetProcessInfoHeader(&DmpHeader.ProcessInfo, &pModuleInfo, 64, ImageFileInfo);

	//. Exception header infomation
	SetExceptionInfoHeader(&DmpHeader.ExceptionInfo, pExceptionInfo);

	//. Context infomation
	memcpy(&DmpHeader.ContextRecord, pContext, sizeof(CONTEXT));
	
	//. Callstack information
	leaf::CCallStackDump CallStackDmp;
	CallStackDmp.Dump(pContext);
	DmpHeader.CallStackDepth = static_cast<USHORT>(CallStackDmp.GetStackDepth());
	
	// Write header
	size_t bytesWritten = fwrite(&DmpHeader, sizeof(DMPFILEHEADER), 1, fd);
	if (bytesWritten != 1) {
		// Handle error
	}
	
	//. Write image file infomation
	if(DmpHeader.ProcessInfo.IsExistFixedFileInfo)
		fwrite(&ImageFileInfo, sizeof(VS_FIXEDFILEINFO), 1, fd);
	
	//. Write modules infomation
	for(int i=0; i<(int)DmpHeader.ProcessInfo.NumOfModules; i++) 
	{
		fwrite(&ModuleInfo[i], sizeof(DMPMODULEINFO), 1, fd);
	}
	
	//. Write callstack information
	for (unsigned int j = 0; j < DmpHeader.CallStackDepth; j++)
	{
		DMPCALLSTACKFRAME DmpStackFrame;
		DmpStackFrame.FrameAddr = (DWORD)CallStackDmp.GetFrameAddr(j);
		DmpStackFrame.ReturnAddr = (DWORD)CallStackDmp.GetReturnAddr(j);
		DWORD pdwParameter[16];
		CallStackDmp.GetParameter(j, pdwParameter, 16);
		memcpy(DmpStackFrame.Parameter, pdwParameter, 16 * sizeof(DWORD));

		fwrite(&DmpStackFrame, sizeof(DMPCALLSTACKFRAME), 1, fd);
	}

	fclose(fd);
	
	return true;
}

bool CExceptionHandler::IsEnableCallback() const
{
	if (m_pfExceptionCallback == NULL)
		return false;
	else
		return true;
}

bool CExceptionHandler::CallExceptionCallback(_EXCEPTION_POINTERS* pExceptionInfo)
{
	if (m_pfExceptionCallback != NULL)
		return m_pfExceptionCallback(pExceptionInfo);
	return false;
}

CExceptionHandler* CExceptionHandler::GetObjPtr() {
	static CExceptionHandler s_Instance;
	return &s_Instance;
}

LONG __stdcall CExceptionHandler::OnException(_EXCEPTION_POINTERS* pExceptionInfo) {
	
	bool bResult;
	if(GetObjPtr()->IsEnableCallback()) {
		bResult = GetObjPtr()->CallExceptionCallback(pExceptionInfo);
	}
	else {
		bResult = SaveExceptionDumpFile(GetObjPtr()->GetDmpFileName(), pExceptionInfo->ContextRecord, pExceptionInfo);
	}

	if(!bResult || GetObjPtr()->IsEnableSecondChance())
		return EXCEPTION_CONTINUE_SEARCH;
	return EXCEPTION_EXECUTE_HANDLER;
}

void CExceptionHandler::SetProcessInfoHeader(DMPPROCESSINFOHEADER* pProcessInfoHeader, DMPMODULEINFO** ppModuleInfo, int MaxModules, VS_FIXEDFILEINFO& ImageFileInfo)
{
	if (pProcessInfoHeader && ppModuleInfo) {
		// Get process information
		pProcessInfoHeader->ProcessId = GetCurrentProcessId();
		char szModuleFileName[256];
		GetModuleFileName(NULL, szModuleFileName, 256);
		lstrcpyn(pProcessInfoHeader->szImageName, szModuleFileName, 256);

		DWORD dwCrc32;
		if (GenerateCrc32Code(pProcessInfoHeader->szImageName, dwCrc32))
			pProcessInfoHeader->CheckSum32 = dwCrc32;

		pProcessInfoHeader->ThreadCount = GetCurrentProcessId(); // assuming the current process only has one thread

		// Get version information
		DWORD dwVersionInfoSize = GetFileVersionInfoSize(pProcessInfoHeader->szImageName, NULL);
		if (dwVersionInfoSize > 0) {
			BYTE* pbyData = new BYTE[dwVersionInfoSize];
			GetFileVersionInfo(pProcessInfoHeader->szImageName, 0, dwVersionInfoSize, pbyData);

			VS_FIXEDFILEINFO* pFileInfoPointer = NULL;
			pProcessInfoHeader->IsExistFixedFileInfo = (BYTE)VerQueryValue(pbyData, "\\",
				(LPVOID*)&pFileInfoPointer, (UINT*)&dwVersionInfoSize);
			if (pProcessInfoHeader->IsExistFixedFileInfo)
				ImageFileInfo = *pFileInfoPointer;

			delete[] pbyData;
		}

		// Get module information
		DWORD NumOfModules = 0;
		HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pProcessInfoHeader->ProcessId);
		if (hModuleSnap != INVALID_HANDLE_VALUE) {
			MODULEENTRY32 ModuleEntry;
			ZeroMemory(&ModuleEntry, sizeof(MODULEENTRY32));

			ModuleEntry.dwSize = sizeof(MODULEENTRY32);
			if (Module32First(hModuleSnap, &ModuleEntry)) {
				do {
					// Only add module info if there is enough space
					if (NumOfModules < MaxModules) {
						DMPMODULEINFO* pModuleInfo = (DMPMODULEINFO*)((*ppModuleInfo) + NumOfModules);
						lstrcpyn(pModuleInfo->szModuleBaseName, ModuleEntry.szModule, 128);
						lstrcpyn(pModuleInfo->szModulePath, ModuleEntry.szExePath, 256);
						pModuleInfo->lpBaseAddr = ModuleEntry.modBaseAddr;
						pModuleInfo->SizeOfModule = ModuleEntry.modBaseSize;

						++NumOfModules;
					}
					else {
						break;
					}

				} while (Module32Next(hModuleSnap, &ModuleEntry));
			}
			pProcessInfoHeader->NumOfModules = NumOfModules;    //. set number of modules.
			CloseHandle(hModuleSnap);
		}
	}
}

void CExceptionHandler::SetExceptionInfoHeader(DMPEXCEPTIONINFOHEADER* pExceptionInfoHeader, _EXCEPTION_POINTERS* pExceptionInfo)
{
	if(pExceptionInfoHeader && pExceptionInfo) {
		pExceptionInfoHeader->ExceptionCode = pExceptionInfo->ExceptionRecord->ExceptionCode;
		pExceptionInfoHeader->ExceptionAddress = (DWORD)pExceptionInfo->ExceptionRecord->ExceptionAddress;
		
		std::string strExceptionCode;
		GetExceptionCodeString(pExceptionInfo->ExceptionRecord, strExceptionCode);
		lstrcpyn(pExceptionInfoHeader->ExceptionCodeString, strExceptionCode.c_str(),128);
	}
}

void CExceptionHandler::GetExceptionCodeString(IN PEXCEPTION_RECORD pExceptionRecord, OUT std::string& strType) {
	switch(pExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		strType = "Access violation";
		if (pExceptionRecord->ExceptionInformation[0] == 0)
		{
			char szExceptionInfo[64];
			sprintf(szExceptionInfo, " : reading inaccessible 0x%08X", pExceptionRecord->ExceptionInformation[1]);
			strType += szExceptionInfo;
		}
		else
		{
			char szExceptionInfo[64];
			sprintf(szExceptionInfo, " : writing inaccessible 0x%08X", pExceptionRecord->ExceptionInformation[1]);
			strType += szExceptionInfo;
		}
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		strType = "Bounds exceeded";
		break;
	case EXCEPTION_BREAKPOINT:
		strType = "Break point";
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		strType = "Data type misalignment";
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		strType = "Float denormal operand";
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		strType = "Float divide by zero";
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		strType = "Float inexact result";
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		strType = "Float invalid operation";
		break;
	case EXCEPTION_FLT_OVERFLOW:
		strType = "Float overflow";
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		strType = "Float stack check";
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		strType = "Float underflow";
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		strType = "Illegal instruction";
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		strType = "In page error";
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		strType = "Int divide by zero";
		break;
	case EXCEPTION_INT_OVERFLOW:
		strType = "Int overflow";
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		strType = "Invalid disposition";
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		strType = "Noncontinuable exception";
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		strType = "Priv instruction";
		break;
	case EXCEPTION_SINGLE_STEP:
		strType = "Single step";
		break;
	case EXCEPTION_STACK_OVERFLOW:
		strType = "Stack overflow";
		break;
	default:
		strType = "Unknown exception";
		break;
	}
}


//. CDmpFileLoader

void CDmpFileLoader::Init() 
{
	ZeroMemory(&m_DmpFileHeader, sizeof(DMPFILEHEADER));
	ZeroMemory(&m_FixedImageFileInfo, sizeof(VS_FIXEDFILEINFO));
}

CDmpFileLoader::CDmpFileLoader()
{ Init(); }
CDmpFileLoader::~CDmpFileLoader()
{ Release(); }

bool CDmpFileLoader::Create(const std::string& dmpfile) 
{	
	if(!m_listModule.empty() || !m_listStackFrame.empty()) { Release(); }
	
	FILE* fd = fopen(dmpfile.c_str(), "rb");
	if(fd == NULL) return false;
	
	fread(&m_DmpFileHeader, sizeof(DMPFILEHEADER), 1, fd);

	if(m_DmpFileHeader.Sign[0] != 'W' || m_DmpFileHeader.Sign[1] != 'D' || m_DmpFileHeader.Sign[2] != 'M' || m_DmpFileHeader.Sign[3] != 'P')
		return false;
	if(m_DmpFileHeader.Version != 0x03)
		return false;
	if(m_DmpFileHeader.HeaderSize != sizeof(DMPFILEHEADER))
		return false;
	
	DMPPROCESSINFOHEADER* pProcessInfoHeader = &m_DmpFileHeader.ProcessInfo;
	//. Read image file infomation
	if(pProcessInfoHeader->IsExistFixedFileInfo == TRUE) {
		fread(&m_FixedImageFileInfo, sizeof(VS_FIXEDFILEINFO), 1, fd);
	}

	//. Read modules infomation

	for(int i=0; i<(int)pProcessInfoHeader->NumOfModules; i++) 
	{
		DMPMODULEINFO* pModuleInfo = new DMPMODULEINFO;
		fread(pModuleInfo, sizeof(DMPMODULEINFO), 1, fd);
		m_listModule.push_back(pModuleInfo);
	}

	for(int j=0; j<(int)m_DmpFileHeader.CallStackDepth; j++) 
	{
		DMPCALLSTACKFRAME* pCallstackFrame = new DMPCALLSTACKFRAME;
		fread(pCallstackFrame, sizeof(DMPCALLSTACKFRAME), 1, fd);
		m_listStackFrame.push_back(pCallstackFrame);
	}
	
	fclose(fd);

	return true;
}
void CDmpFileLoader::Release() {
	if(!m_listModule.empty()) {	//. Release module list
		t_modulevect::iterator mviter = m_listModule.begin();
		for(; mviter != m_listModule.end(); mviter++)
			delete (*mviter);
		m_listModule.clear();
	}
	if(!m_listStackFrame.empty()) {	//. Release callstack frame list
		t_framevect::iterator fviter = m_listStackFrame.begin();
		for(; fviter != m_listStackFrame.end(); fviter++)
			delete (*fviter);
		m_listStackFrame.clear();
	}
	Init();
}

DWORD CDmpFileLoader::GetCreationTimeStamp() const
{ return m_DmpFileHeader.CreationTimeStamp; }


//. Process infomation
DWORD CDmpFileLoader::GetProcessId() const
{ return m_DmpFileHeader.ProcessInfo.ProcessId; }
DWORD CDmpFileLoader::GetCheckSum32() const
{ return m_DmpFileHeader.ProcessInfo.CheckSum32; }
DWORD CDmpFileLoader::GetThreadCount() const
{ return m_DmpFileHeader.ProcessInfo.ThreadCount; }
const char* CDmpFileLoader::GetImageName() const
{ return m_DmpFileHeader.ProcessInfo.szImageName; }
const VS_FIXEDFILEINFO* CDmpFileLoader::GetFixedImageFileInfo() const
{ return &m_FixedImageFileInfo; }

//. Modules infomation
size_t CDmpFileLoader::GetNumOfModules() const
{ return m_listModule.size(); }
const DMPMODULEINFO* CDmpFileLoader::GetModuleInfo(int index) const
{
	if(index >= 0 && index < (int)GetNumOfModules())
		return m_listModule[index];
	return NULL;
}

//. Exception infomation
const char* CDmpFileLoader::GetOSInfoString() const 
{ return m_DmpFileHeader.OSInfoString; }
const char* CDmpFileLoader::GetCPUInfoString() const 
{ return m_DmpFileHeader.CPUInfoString; }
const MEMORYSTATUS& CDmpFileLoader::GetMemoryStatus() const
{ return m_DmpFileHeader.MemStatus; }

const CONTEXT& CDmpFileLoader::GetContextRecord() const
{ return m_DmpFileHeader.ContextRecord; }
DWORD CDmpFileLoader::GetExceptionCode() const 
{ return m_DmpFileHeader.ExceptionInfo.ExceptionCode; }
DWORD CDmpFileLoader::GetExceptionAddress() const 
{ return m_DmpFileHeader.ExceptionInfo.ExceptionAddress; }
const char* CDmpFileLoader::GetExceptionCodeString() const 
{ return m_DmpFileHeader.ExceptionInfo.ExceptionCodeString; }

//. Callstack infomation
size_t CDmpFileLoader::GetStackDepth() const 
{ return m_listStackFrame.size(); }
const DMPCALLSTACKFRAME* CDmpFileLoader::GetCallStackFrame(int index) const
{
	if(index >=0 && index < (int)GetStackDepth())
		return m_listStackFrame[index];
	return NULL;
}