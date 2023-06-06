
#pragma once

#include "include.h"

class CStringToken  
{
public:
	CStringToken();
	virtual ~CStringToken();
	CStringToken(const std::string& dataLine, const std::string& delim);
	
	size_t countTokens();						//��ū�� ���� 
	bool hasMoreTokens();						//��ū�� �����ϴ��� Ȯ�� 
	std::string nextToken();					//���� ��ū 

private: 
	std::string data;
	std::string delimiter;						//������, ������ 
	std::vector<std::string> tokens;			//��ū�� ���Ϳ� ���� 
	std::vector<std::string>::iterator index;	//���Ϳ� ���� �ݺ��� 

	void split();								//��Ʈ���� �����ڷ� ������ ���Ϳ� ���� 
	void IsNullString(std::string::size_type pos);						//��ū�� ���� ������ ���Ϳ� �ΰ� �־��ֱ�
};
