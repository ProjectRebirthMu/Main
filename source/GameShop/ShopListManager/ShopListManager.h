
/*
�ۼ���: 2009-07-24
�ۼ���: ������
���: ������Ʈ ������ ���� ���Ǵ� ��ü
*/

#pragma once

#include "include.h"
#include "ListManager.h"
#include "ShopList.h"

class CShopListManager : public CListManager
{
public:
	CShopListManager();	
	virtual ~CShopListManager();

	CShopList*		GetListPtr() {return m_ShopList;};

private:
	CShopList*		m_ShopList;

	WZResult		LoadScript(bool bDonwLoad);
};