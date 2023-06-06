// TestEventMap.cpp: implementation of the TestEventMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestEventMap.h"

#ifdef PSW_ADD_TESTMAP

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TestEventMapPtr	TestEventMap::Make()
{
	TestEventMapPtr map( new TestEventMap );
	return map;
}

TestEventMap::TestEventMap() : m_IsTest( true )
{

}

TestEventMap::~TestEventMap()
{

}


// ������Ʈ ����
bool TestEventMap::CreateObject(OBJECT* o)
{
	return false;
}

// ������Ʈ ���μ���
bool TestEventMap::MoveObject(OBJECT* o)
{
	return false;
}

// ������Ʈ ����Ʈ
bool TestEventMap::RenderObjectVisual(OBJECT* o, BMD* b)
{
	return false;
}

// ������Ʈ �Ž� ����Ʈ
bool TestEventMap::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	return false;
}

// �� ���� ������Ʈ ����Ʈ
void TestEventMap::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{

}

// ���� ����
CHARACTER* TestEventMap::CreateMonster(int iType, int PosX, int PosY, int Key)
{
	CHARACTER* pCharacter = NULL;
	return pCharacter;
}

// ����(NPC) ���μ���
bool TestEventMap::MoveMonsterVisual(OBJECT* o, BMD* b)
{
	return false;
}
// ���� ��ų �� ����Ʈ
void TestEventMap::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{

}

// ���� ����Ʈ ( �Ϲ� )	
bool TestEventMap::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	return false;
}

// ���� ����Ʈ ( ��ų )
bool TestEventMap::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
	return false;
}

// ��ų �ִϸ��̼� ���� �Լ�
bool TestEventMap::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
	return false;
}

bool TestEventMap::PlayMonsterSound(OBJECT* o)
{
	return false;
}

void TestEventMap::PlayObjectSound(OBJECT* o)
{

}

#endif //PSW_ADD_TESTMAP