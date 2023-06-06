// DuelMgr.cpp: implementation of the CDuelMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DuelMgr.h"
#include "ZzzBMD.h"
#include "ZzzEffect.h"
#include "ZzzObject.h"
#include "ZzzInventory.h"
#include "ZzzInterface.h"
#include "ZzzInfomation.h"
#include "ZzzCharacter.h"
#include "wsclientinline.h"
#include "CharacterManager.h"

CDuelMgr g_DuelMgr;

CDuelMgr::CDuelMgr()
{
	Reset();
}

CDuelMgr::~CDuelMgr()
{
	Reset();
}

void CDuelMgr::Reset()
{
	m_bIsDuelEnabled = FALSE;
	m_bIsPetDuelEnabled = FALSE;
	memset(m_DuelPlayer, 0, sizeof(DUEL_PLAYER_INFO) * MAX_DUEL_PLAYERS);
	memset(m_DuelChannels, 0, sizeof(DUEL_CHANNEL_INFO) * MAX_DUEL_CHANNELS);
	m_iCurrentChannel = -1;
	m_bRegenerated = FALSE;
	RemoveAllDuelWatchUser();
}

void CDuelMgr::EnableDuel(BOOL bEnable)
{
	m_bIsDuelEnabled = bEnable;

	if (bEnable == FALSE)
	{
		Reset();
	}
}

BOOL CDuelMgr::IsDuelEnabled()
{
	return m_bIsDuelEnabled;
}

void CDuelMgr::EnablePetDuel(BOOL bEnable)
{
	m_bIsPetDuelEnabled = bEnable;
}

BOOL CDuelMgr::IsPetDuelEnabled()
{
	return m_bIsPetDuelEnabled;
}

void CDuelMgr::SetDuelPlayer(int iPlayerNum, int iIndex, char * pszID)
{
	m_DuelPlayer[iPlayerNum].m_iIndex = iIndex;
	strncpy(m_DuelPlayer[iPlayerNum].m_szID, pszID, MAX_ID_SIZE);
	m_DuelPlayer[iPlayerNum].m_szID[MAX_ID_SIZE] = '\0';
}

void CDuelMgr::SetHeroAsDuelPlayer(int iPlayerNum)
{
	m_DuelPlayer[iPlayerNum].m_iIndex = Hero->Key;
	strncpy(m_DuelPlayer[iPlayerNum].m_szID, Hero->ID, MAX_ID_SIZE);
	m_DuelPlayer[iPlayerNum].m_szID[MAX_ID_SIZE] = '\0';
}

void CDuelMgr::SetScore(int iPlayerNum, int iScore)
{
	m_DuelPlayer[iPlayerNum].m_iScore = iScore;
}

void CDuelMgr::SetHP(int iPlayerNum, int iRate)
{
	m_DuelPlayer[iPlayerNum].m_fHPRate = iRate * 0.01f;
}

void CDuelMgr::SetSD(int iPlayerNum, int iRate)
{
	m_DuelPlayer[iPlayerNum].m_fSDRate = iRate * 0.01f;
}

char * CDuelMgr::GetDuelPlayerID(int iPlayerNum)
{
	return m_DuelPlayer[iPlayerNum].m_szID;
}

int CDuelMgr::GetScore(int iPlayerNum)
{
	return m_DuelPlayer[iPlayerNum].m_iScore;
}

float CDuelMgr::GetHP(int iPlayerNum)
{
	return m_DuelPlayer[iPlayerNum].m_fHPRate;
}

float CDuelMgr::GetSD(int iPlayerNum)
{
	return m_DuelPlayer[iPlayerNum].m_fSDRate;
}

BOOL CDuelMgr::IsDuelPlayer(CHARACTER * pCharacter, int iPlayerNum, BOOL bIncludeSummon)
{
	if (pCharacter->Key == m_DuelPlayer[iPlayerNum].m_iIndex && strncmp(pCharacter->ID, m_DuelPlayer[iPlayerNum].m_szID, MAX_ID_SIZE) == 0)
	{
		return TRUE;
	}
	else if (bIncludeSummon == TRUE && gCharacterManager.GetBaseClass(pCharacter->Class) == 0 && strncmp(pCharacter->OwnerID, m_DuelPlayer[iPlayerNum].m_szID, MAX_ID_SIZE) == 0)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CDuelMgr::IsDuelPlayer(WORD wIndex, int iPlayerNum)
{
	return (m_DuelPlayer[iPlayerNum].m_iIndex == wIndex);
}

void CDuelMgr::SendDuelRequestAnswer(int iPlayerNum, BOOL bOK)
{
	SendRequestDuelOk(bOK, m_DuelPlayer[iPlayerNum].m_iIndex, m_DuelPlayer[iPlayerNum].m_szID);
}

void CDuelMgr::SetDuelChannel(int iChannelIndex, BOOL bEnable, BOOL bJoinable, char * pszID1, char * pszID2)
{
	m_DuelChannels[iChannelIndex].m_bEnable = bEnable;
	m_DuelChannels[iChannelIndex].m_bJoinable = bJoinable;
	strncpy(m_DuelChannels[iChannelIndex].m_szID1, pszID1, MAX_ID_SIZE);
	m_DuelChannels[iChannelIndex].m_szID1[MAX_ID_SIZE] = '\0';
	strncpy(m_DuelChannels[iChannelIndex].m_szID2, pszID2, MAX_ID_SIZE);
	m_DuelChannels[iChannelIndex].m_szID2[MAX_ID_SIZE] = '\0';
}

void CDuelMgr::RemoveAllDuelWatchUser()
{
	m_DuelWatchUserList.clear();
}

void CDuelMgr::AddDuelWatchUser(char * pszUserID)
{
	char szUserID[24] = { 0, };
	strncpy(szUserID, pszUserID, MAX_ID_SIZE);
	szUserID[MAX_ID_SIZE] = '\0';
	m_DuelWatchUserList.push_back(szUserID);
}

void CDuelMgr::RemoveDuelWatchUser(char * pszUserID)
{
	std::list<std::string>::iterator iter;
	for (iter = m_DuelWatchUserList.begin(); iter != m_DuelWatchUserList.end(); ++iter)
	{
		if (strncmp(iter->c_str(), pszUserID, MAX_ID_SIZE) == 0)
		{
			m_DuelWatchUserList.erase(iter);
			return;
		}
	}
	assert(!"RemoveDuelWatchUser!");
}

char * CDuelMgr::GetDuelWatchUser(int iIndex)
{
	if(m_DuelWatchUserList.size() <= (DWORD)iIndex)
		return NULL;

	int i = 0;
	std::list<std::string>::iterator iter;
	for (iter = m_DuelWatchUserList.begin(); iter != m_DuelWatchUserList.end(); ++iter, ++i)
	{
		if (i == iIndex)
		{
			return (char *)iter->c_str();
		}
	}
	return NULL;
}
