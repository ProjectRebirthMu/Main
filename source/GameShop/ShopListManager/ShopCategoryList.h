
/**************************************************************************************************

��ü ī�װ� ��� ��ü

iterator�� �̿��Ͽ� ���������� ī�װ� ��ü�� ������ �� �ִ�.
ī�װ� ��ȣ�� �̿��Ͽ� ī�װ� ��ü�� ������ �� �ִ�.

**************************************************************************************************/

#pragma once

#include "ShopCategory.h"
#include <map>

class CShopCategoryList
{
public:
	CShopCategoryList(void);
	~CShopCategoryList(void);

	void Clear();
	
	int GetSize();
	virtual void Append(CShopCategory category);

	void SetFirst();											// ī�װ� ��Ͽ��� ù ��° ī�װ��� ����Ű�� �Ѵ�.
	bool GetNext(CShopCategory& category);						// ���� ī�װ� ��ü�� �ѱ�� ���� ī�װ� ��ü�� ����Ű�� �Ѵ�.

	bool GetValueByKey(int nKey, CShopCategory& category);		// ī�װ� �������� ī�װ� ��ü�� �����´�.
	bool GetValueByIndex(int nIndex, CShopCategory& category);	// �ε��� ��ȣ�� ī�װ� ��ü�� �����´�. 

	bool InsertPackage(int Category, int Package);
	bool RefreshPackageSeq(int Category, int PackageSeqs[], int PackageCount);

protected:
	std::map<int, CShopCategory> m_Categroys;				// ī�װ� ��ü ��
	std::map<int, CShopCategory>::iterator m_Categoryiter;	// ī�װ� iterator
	std::vector<int> m_CategoryIndex;						// ī�װ� ��ȣ �ε��� ���

};
