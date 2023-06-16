// BoneManager
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BoneManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzInterface.h"
#include "./Utilities/Log/ErrorReport.h"


void BoneManager::RegisterBone(CHARACTER* pCharacter, const std::string& name, int nBone)
{
	CBoneManager::GetInstance()->RegisterBone(pCharacter, name, nBone);
}

void BoneManager::UnregisterBone(CHARACTER* pCharacter, const std::string& name)
{
	CBoneManager::GetInstance()->UnregisterBone(pCharacter, name);
}

void BoneManager::UnregisterBone(CHARACTER* pCharacter)
{
	CBoneManager::GetInstance()->UnregisterBone(pCharacter);
}

void BoneManager::UnregisterAll()
{
	CBoneManager::GetInstance()->UnregisterAll();
}

CHARACTER* BoneManager::GetOwnCharacter(OBJECT* pObject, const std::string& name)
{
	return CBoneManager::GetInstance()->GetOwnCharacter(pObject, name);
}

int BoneManager::GetBoneNumber(OBJECT* pObject, const std::string& name)
{
	return CBoneManager::GetInstance()->GetBoneNumber(pObject, name);
}

bool BoneManager::GetBonePosition(OBJECT* pObject, const std::string& name, OUT vec3_t Position)
{
	return CBoneManager::GetInstance()->GetBonePosition(pObject, name, Position);
}

bool BoneManager::GetBonePosition(OBJECT* pObject, const std::string& name, IN vec3_t Relative, OUT vec3_t Position)
{
	return CBoneManager::GetInstance()->GetBonePosition(pObject, name, Relative, Position);
}

CBoneManager::CBoneManager()
{
}

CBoneManager::~CBoneManager()
{
	UnregisterAll();
}

void CBoneManager::RegisterBone(CHARACTER* pCharacter, const std::string& name, int nBone)
{
	if (pCharacter)
	{
		BMD* pModel = &Models[pCharacter->Object.Type];
		if (pModel)
		{
			if (FindBone(&(pCharacter->Object), name))
				UnregisterBone(pCharacter, name);

			BONEINFO* pBoneInfo = new BONEINFO;
			pBoneInfo->name = name;
			pBoneInfo->pCharacter = pCharacter;
			pBoneInfo->pModel = pModel;
			pBoneInfo->nBone = nBone;

			m_listBone.push_back(pBoneInfo);
		}
	}
}

void CBoneManager::UnregisterBone(CHARACTER* pCharacter, const std::string& name)
{
	if (pCharacter)
	{
		BONEINFO* pBoneInfo = FindBone(&(pCharacter->Object), name);
		if (pBoneInfo)
		{
			m_listBone.remove(pBoneInfo);
			delete pBoneInfo;
		}
	}
}

void CBoneManager::UnregisterBone(CHARACTER* pCharacter)
{
	for (auto iter = m_listBone.begin(); iter != m_listBone.end(); )
	{
		BONEINFO* pBoneInfo = (*iter);
		if (pBoneInfo->pCharacter == pCharacter)
		{
			delete pBoneInfo;
			iter = m_listBone.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void CBoneManager::UnregisterAll()
{
	for (auto iter = m_listBone.begin(); iter != m_listBone.end(); ++iter)
	{
		delete (*iter);
	}

	m_listBone.clear();
}

CHARACTER* CBoneManager::GetOwnCharacter(OBJECT* pObject, const std::string& name)
{
	BONEINFO* pBoneInfo = FindBone(pObject, name);
	if (pBoneInfo)
	{
		return pBoneInfo->pCharacter;
	}
	return nullptr;
}

int CBoneManager::GetBoneNumber(OBJECT* pObject, const std::string& name)
{
	LPBONEINFO _lpBoneInfo = FindBone(pObject, name);
	if(_lpBoneInfo)
		return _lpBoneInfo->nBone;
	return -1;
}

bool CBoneManager::GetBonePosition(OBJECT* pObject, const std::string& name, OUT vec3_t Position)
{
	LPBONEINFO _lpBoneInfo = FindBone(pObject, name);
	if(_lpBoneInfo) {
		vec3_t LocalPos, Relative;
		Vector ( 0.f, 0.f, 0.f, Relative );
		_lpBoneInfo->pModel->TransformPosition(pObject->BoneTransform[_lpBoneInfo->nBone], Relative, LocalPos, false );
		VectorScale(LocalPos,pObject->Scale,LocalPos);
		VectorAdd(LocalPos, pObject->Position, Position);
		return true;
	}
	return false;
}

bool CBoneManager::GetBonePosition(OBJECT* pObject, const std::string& name, IN vec3_t Relative, OUT vec3_t Position)
{
	LPBONEINFO _lpBoneInfo = FindBone(pObject, name);
	if(_lpBoneInfo) {
		vec3_t LocalPos;
		_lpBoneInfo->pModel->TransformPosition(pObject->BoneTransform[_lpBoneInfo->nBone], Relative, LocalPos, false );
		VectorScale(LocalPos,pObject->Scale,LocalPos);
		VectorAdd(LocalPos, pObject->Position, Position);
		return true;
	}
	return false;
}

CBoneManager* CBoneManager::GetInstance()
{
	static CBoneManager s_Instace;
	return &s_Instace;
}

CBoneManager::LPBONEINFO CBoneManager::FindBone(OBJECT* pObject, const std::string& name)
{
	for (auto iter = m_listBone.begin(); iter != m_listBone.end(); ++iter)
	{
		LPBONEINFO _lpBoneInfo = (*iter);
		if ((&_lpBoneInfo->pCharacter->Object == pObject) && (_lpBoneInfo->name == name))
			return _lpBoneInfo;
	}
	return nullptr;
}