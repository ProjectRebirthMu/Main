
#pragma once

#include "include.h"
#include <map>

class CBannerInfo
{
public:
	CBannerInfo();
	virtual ~CBannerInfo();

	bool	SetBanner(std::string strdata, std::string strDirPath, bool bDonwLoad);

public:	
	int		BannerSeq;									//  1. ��� �׷� ����
	char	BannerName[BANNER_LENGTH_NAME];				//  2. ��� �׷� ��
	char	BannerImageURL[INTERNET_MAX_URL_LENGTH];	//  3. ��� �̹��� URL
	int		BannerOrder;								//  4. ��� ���� ����
	int		BannerDirection;							//  5. ��� ���� ���� * ���� �ڵ� ���� ����
	tm 		BannerStartDate;							//  6. ��� ���� ������
	tm 		BannerEndDate;								//  7. ��� ���� ������
	char	BannerLinkURL[INTERNET_MAX_URL_LENGTH];		//  8. ��� ��ũ URL

	char	BannerImagePath[MAX_PATH];					// ��� �̹��� ���� ���� ���
};
