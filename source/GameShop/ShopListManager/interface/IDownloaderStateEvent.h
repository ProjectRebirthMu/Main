/*******************************************************************************
*	�� �� �� : ������
*	�� �� �� : 2009.06.10
*	��    �� : Interface - Download State Event ���� Ŭ����
*				�ٿ�ε� ���¸� ���� ���� ��ü
*				���̺귯���� ����� ���α׷����� �������Ͽ� ���
*******************************************************************************/

#pragma once

class IDownloaderStateEvent
{
public:
// Constructor, Destructor

	IDownloaderStateEvent(){};
	virtual ~IDownloaderStateEvent(){};


// abstract Function

	//				�ٿ�ε� ���� �̺�Ʈ �ڵ鷯
	virtual void	OnStartedDownloadFile(TCHAR* szFileName, ULONGLONG uFileLength) = 0;
	//				�ٿ�ε� ���� ��Ȳ �̺�Ʈ �ڵ鷯 : ��Ŷ ����
	virtual void	OnProgressDownloadFile(TCHAR* szFileName, ULONGLONG uDownloadFileLength) = 0;
	//				�ٿ�ε� ���� �̺�Ʈ �ڵ鷯
	virtual void	OnCompletedDownloadFile(TCHAR* szFileName, WZResult wzResult) = 0;
};

