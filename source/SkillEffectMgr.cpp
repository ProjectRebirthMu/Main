//////////////////////////////////////////////////////////////////////
// SkillEffectMgr.cpp: implementation of the CSkillEffectMgr class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkillEffectMgr.h"
#include "ZzzLodTerrain.h"
#include "CDirection.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"

CSkillEffectMgr g_SkillEffects;

void MoveEffect( OBJECT *o, int iIndex);

CSkillEffectMgr::CSkillEffectMgr()
{
}

CSkillEffectMgr::~CSkillEffectMgr()
{
}

OBJECT * CSkillEffectMgr::GetEffect(int iIndex)
{
	assert(iIndex < MAX_SKILL_EFFECTS);

	return m_SkillEffects + iIndex;
}

BOOL CSkillEffectMgr::IsSkillEffect(int Type, vec3_t Position, vec3_t Angle,
	vec3_t Light, int SubType, OBJECT* Owner, short PKKey, WORD SkillIndex, WORD Skill, WORD SkillSerialNum,
	float Scale, short int sTargetIndex) {
	if (Owner != &Hero->Object)
		return FALSE;
	switch (Type) {
	case BITMAP_BOSS_LASER:
	case MODEL_SKILL_BLAST:
	case MODEL_DARK_SCREAM:
	case BITMAP_SWORD_FORCE:
	case MODEL_SKILL_INFERNO:
		return TRUE;
	case MODEL_CIRCLE:
		if (SubType == 0) return TRUE;
		break;
	case BITMAP_FLAME:
		if (SubType == 0) return TRUE;
		break;
	case MODEL_STORM:
		if (SubType == 0) return TRUE;
		break;
	}

	return FALSE;
}

OBJECT* CSkillEffectMgr::CreateEffect()
{
	for (int icntEffect = 0; icntEffect < MAX_SKILL_EFFECTS; icntEffect++)
	{
		OBJECT* o = &m_SkillEffects[icntEffect];
		if (!o->Live)
		{
			return o;
		}
	}
	assert(!"No available SkillEffect Object!");
	return &m_SkillEffects[MAX_SKILL_EFFECTS - 1];
}

bool CSkillEffectMgr::DeleteEffect(int Type, OBJECT* Owner, int iSubType)
{
	bool bDelete = false;
	for (int i = 0; i < MAX_SKILL_EFFECTS; i++)
	{
		OBJECT* o = &m_SkillEffects[i];
		if (o->Live && o->Type == Type)
		{
			if (iSubType == -1 || iSubType == o->SubType)
			{
				if (o->Owner == Owner)
				{
					EffectDestructor(o);
					bDelete = true;
				}
			}
		}
	}
	return bDelete;
}

void CSkillEffectMgr::DeleteEffect(int efftype)
{
	for (int i = 0; i < MAX_SKILL_EFFECTS; i++)
	{
		OBJECT* o = &m_SkillEffects[i];
		if (o->Live && o->Type == efftype)
		{
			EffectDestructor(o);
		}
	}
}

bool CSkillEffectMgr::SearchEffect(int iType, OBJECT* pOwner, int iSubType)
{
	for(int i=0; i<MAX_SKILL_EFFECTS; ++i)
	{
		OBJECT *o = &m_SkillEffects[i];
		if(o->Live && o->Type == iType && o->Owner == pOwner)
		{
			if(iSubType == -1 || o->SubType == iSubType)
			{
				return true;
			}
		}
	}

    return false;
}

BOOL CSkillEffectMgr::FindSameEffectOfSameOwner(int iType, OBJECT* pOwner)
{
	for (int i = 0; i < MAX_SKILL_EFFECTS; ++i)
	{
		OBJECT* o = &m_SkillEffects[i];
		if (o->Live && o->Type == iType && o->Owner == pOwner)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CSkillEffectMgr::MoveEffects()
{
	for(int i=0;i<MAX_SKILL_EFFECTS;i++)
	{
		OBJECT *o = &m_SkillEffects[i];
		if(o->Live)
		{
			MoveEffect( o, i);
		}
	}
}
