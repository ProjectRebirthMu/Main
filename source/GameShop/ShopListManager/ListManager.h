
/**************************************************************************************************

�ۼ���: 2009.10.06
�ۼ���: ������

**************************************************************************************************/

#pragma once
#include "include.h"
#include "FTPFileDownLoader.h"

class CListManager  
{
public:
	CListManager();	
	virtual ~CListManager();

	//				�� ����Ʈ �ٿ�ε� & ���� �� ���� ������ �����Ѵ�.
	void			SetListManagerInfo(DownloaderType type, 
									   char* ServerIP, 
									   char* UserID, 
									   char* Pwd, 
									   char* RemotePath, 
									   char* LocalPath, 
									   CListVersionInfo Version,
									   DWORD dwDownloadMaxTime = 0);

	void			SetListManagerInfo(DownloaderType type, 
									   char* ServerIP, 
									   unsigned short PortNum, 
									   char* UserID, 
									   char* Pwd, 
									   char* RemotePath, 
									   char* LocalPath, 
									   FTP_SERVICE_MODE ftpMode, 
									   CListVersionInfo Version,
									   DWORD dwDownloadMaxTime = 0);

	WZResult		LoadScriptList(bool bDonwLoad);

protected:
	bool			IsScriptFileExist();
	std::string		GetScriptPath();
	void			DeleteScriptFiles();

	WZResult		FileDownLoad();
	WZResult		FileDownLoadImpl();
	static unsigned int __stdcall RunFileDownLoadThread(LPVOID pParam);

	virtual WZResult LoadScript(bool bDonwLoad) = 0;

	CListManagerInfo			m_ListManagerInfo;
	std::vector<std::string>	m_vScriptFiles;
	WZResult					m_Result;
	CFTPFileDownLoader*			m_pFTPDownLoader;
};
