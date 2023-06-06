// TestEventMap.h: interface for the TestEventMap class.
//
//////////////////////////////////////////////////////////////////////

#ifdef PSW_ADD_TESTMAP

#if !defined(AFX_TESTEVENTMAP_H__9ABFF262_D93D_4B3C_9620_681662E7A384__INCLUDED_)
#define AFX_TESTEVENTMAP_H__9ABFF262_D93D_4B3C_9620_681662E7A384__INCLUDED_

#pragma once

#include "w_BaseMap.h"

BoostSmartPointer( TestEventMap );
class TestEventMap : public BaseMap
{
public:
	static TestEventMapPtr	Make();
	virtual ~TestEventMap();	

public:	// Object
	// ������Ʈ ����
	virtual bool CreateObject(OBJECT* o);
	// ������Ʈ ���μ���
	virtual bool MoveObject(OBJECT* o);
	// ������Ʈ ����Ʈ
	virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
	// ������Ʈ �Ž� ����Ʈ
	virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
	// �� ���� ������Ʈ ����Ʈ
	virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
	
public:	// Character
	// ���� ����
	virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
	// ����(NPC) ���μ���
	virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
	// ���� ��ų �� ����Ʈ
	virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
	// ���� ����Ʈ ( �Ϲ� )	
	virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
	// ���� ����Ʈ ( ��ų )
	virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
	// ��ų �ִϸ��̼� ���� �Լ�
	virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);

public:
	virtual bool PlayMonsterSound(OBJECT* o);
	virtual void PlayObjectSound(OBJECT* o);

public:
	const bool GetTest() const;

public:
	TestEventMap();

private:
	bool m_IsTest;

};

inline
const bool TestEventMap::GetTest() const
{
	return m_IsTest;
}

#endif // !defined(AFX_TESTEVENTMAP_H__9ABFF262_D93D_4B3C_9620_681662E7A384__INCLUDED_)

#endif //PSW_ADD_TESTMAP
