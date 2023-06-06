/*******************************************************************************
*	�� �� �� : ������
*	�� �� �� : 2009.06.10
*	��    �� : ��Ÿ �޼ҵ�
*******************************************************************************/

#pragma once

class Path
{
public:

	//					��� ��ü ��� ��������
	static TCHAR*		GetCurrentFullPath(TCHAR* szPath);
	//					��� ���丮 ��������
	static TCHAR*		GetCurrentDirectory(TCHAR* szPath);
	//					��� ���� �̸� ��������
	static TCHAR*		GetCurrentFileName(TCHAR* szPath);

	//					���� ���ڿ� ����� : �� �ڿ� "\\" �ٿ��ش�.
	static TCHAR*		SetDirString(TCHAR * szPath);
	//					���� ���ڿ� ����� : �� �ڿ� "\\" ����
	static TCHAR*		ClearDirString(TCHAR * szPath);
	
	//					���� ���ڿ� ����� : ���ϸ� ������ ���
	static TCHAR*		GetDirectory(TCHAR * szPath);
	//					���� ���ڿ� ����� : �н� ������ ���� ��
	static TCHAR*		GetFileName(TCHAR * szPath);

	//					/ => \\ �� ����
	static TCHAR*		ChangeSlashToBackSlash(TCHAR * szPath);
	//					\\ => / �� ����
	static TCHAR*		ChangeBackSlashToSlash(TCHAR * szPath);

	//					���Ͽ��� ������ �� �о����
	static BOOL			ReadFileLastLine(TCHAR * szFile, TCHAR * szLastLine);
	//					�� ���Ͽ� ���� ����
	static BOOL			WriteNewFile(TCHAR * szFile, TCHAR * szText, INT nTextSize);
	//					���� ��� ���丮 ����
	static BOOL			CreateDirectorys(TCHAR * szFilePath, BOOL bIsFile);
};

