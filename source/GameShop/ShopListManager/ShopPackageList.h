
/**************************************************************************************************

��ü ��Ű�� ��� ��ü

iterator�� �̿��Ͽ� ���������� ��Ű�� ��ü�� ������ �� �ִ�.
��Ű�� ��ȣ�� �̿��Ͽ� ��Ű�� ��ü�� ������ �� �ִ�.

**************************************************************************************************/

#pragma once

#include "ShopPackage.h"
#include <map>

class CShopPackageList
{
public:
	CShopPackageList(void);
	~CShopPackageList(void);

	int GetSize();
	void Clear();

	virtual void Append(CShopPackage package);

	void SetFirst();											// ��Ű�� ��Ͽ��� ù ��° ��Ű���� ����Ű�� �Ѵ�.
	bool GetNext(CShopPackage& package);						// ���� ��Ű�� ��ü�� �ѱ�� ���� ��Ű�� ��ü�� ����Ű�� �Ѵ�.
	
	bool GetValueByKey(int nKey, CShopPackage& package);		// ��Ű�� ��ȣ�� �ش� ��Ű�� ��ü ��������
	bool GetValueByIndex(int nIndex, CShopPackage& package);	// �ε��� ��ȣ�� �ش� ��Ű�� ��ü ��������
	
	bool SetPacketLeftCount(int PackageSeq, int nCount);

protected:
	std::map<int, CShopPackage> m_Packages;
	std::map<int, CShopPackage>::iterator m_iter;
	std::vector<int> m_PackageIndex;
};
