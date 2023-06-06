
/**************************************************************************************************

��ǰ(�Ӽ�) ��ü

���� ��ǰ�� �⺻ ������ �� ���� �Ӽ� ������ ������ �ִ�.
��ǰ ��ȣ�� ������ ���� �Ӽ��� �����Ͽ� �� ���� ��ǰ�� ǥ���ؾ� �Ѵ�.

**************************************************************************************************/

#pragma once

#include "include.h"
#include <map>

class CShopProduct
{
public:
	CShopProduct();
	virtual ~CShopProduct();

	bool SetProduct(std::string strdata);

public:	
	int		ProductSeq;											//  1. ��ǰ ��ȣ
	char	ProductName[SHOPLIST_LENGTH_PRODUCTNAME];			//  2. ��ǰ ��
	char	PropertyName[SHOPLIST_LENGTH_PRODUCTPROPERTYNAME];	//  3. �Ӽ� ��
	char	Value[SHOPLIST_LENGTH_PRODUCTVALUE];				//  4. �Ӽ� ��
	char	UnitName[SHOPLIST_LENGTH_PRODUCTUNITNAME];			//  5. �Ӽ� ���� ��
	int		Price;												//  6. ��ǰ ����
	int		PriceSeq;											//  7. ��ǰ ���� ��ȣ
	int		PropertyType;										//  8. �Ӽ� ���� (141:������ �Ӽ�, 142:���� �Ӽ�)
	int		MustFlag;											//  9. �ʼ� ���� (145:�ʼ�, 146:����)
	int		vOrder;												// 10. ���� �Ӽ� ���� (1:���� �Ӽ�, 9:���� �Ӽ�)
	int		DeleteFlag;											// 11. ���� ���� (143: ����, 144: Ȱ��)
	int		StorageGroup;										// 12. ������ �׷� ����
	int		ShareFlag;											// 13. ServerType(���� ����) �� ������ ���� ���� �׸� ����
	char	InGamePackageID[SHOPLIST_LENGTH_INGAMEPACKAGEID];	// 14. ������ �ڵ�
	int		PropertySeq;										// 15. �Ӽ� �ڵ�
	int		ProductType;										// 16. ��ǰ ���� �ڵ�
	int		UnitType;											// 17. ���� �ڵ�
};
