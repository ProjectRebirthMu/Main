// DuelMgr.h: interface for the CDuelMgr class.
//////////////////////////////////////////////////////////////////////
#pragma once


typedef struct _DUEL_PLAYER_INFO
{
	int m_iIndex;
	char m_szID[24];
	int m_iScore;
	float m_fHPRate;
	float m_fSDRate;
} DUEL_PLAYER_INFO;

enum _DUEL_PLAYER_TYPE
{
	DUEL_HERO,
	DUEL_ENEMY,
	MAX_DUEL_PLAYERS
};

typedef struct _DUEL_CHANNEL_INFO
{
	BOOL m_bEnable;
	BOOL m_bJoinable;
	char m_szID1[24];
	char m_szID2[24];
} DUEL_CHANNEL_INFO;

#define MAX_DUEL_CHANNELS 4

class CDuelMgr  
{
public:
	CDuelMgr();
	virtual ~CDuelMgr();
	void Reset();
public:
	void EnableDuel(BOOL bEnable);
	BOOL IsDuelEnabled();

	void EnablePetDuel(BOOL bEnable);
	BOOL IsPetDuelEnabled();

	void SetDuelPlayer(int iPlayerNum, int iIndex, char * pszID);
	void SetHeroAsDuelPlayer(int iPlayerNum);
	void SetScore(int iPlayerNum, int iScore);
	void SetHP(int iPlayerNum, int iRate);
	void SetSD(int iPlayerNum, int iRate);

	char * GetDuelPlayerID(int iPlayerNum);
	int GetScore(int iPlayerNum);
	float GetHP(int iPlayerNum);
	float GetSD(int iPlayerNum);

	BOOL IsDuelPlayer(CHARACTER * pCharacter, int iPlayerNum, BOOL bIncludeSummon = TRUE);
	BOOL IsDuelPlayer(WORD wIndex, int iPlayerNum);

	void SendDuelRequestAnswer(int iPlayerNum, BOOL bOK);

protected:
	BOOL m_bIsDuelEnabled;
	BOOL m_bIsPetDuelEnabled;
	DUEL_PLAYER_INFO m_DuelPlayer[MAX_DUEL_PLAYERS];

public:
	void SetDuelChannel(int iChannelIndex, BOOL bEnable, BOOL bJoinable, char * pszID1, char * pszID2);
	BOOL IsDuelChannelEnabled(int iChannelIndex) { return m_DuelChannels[iChannelIndex].m_bEnable; }
	BOOL IsDuelChannelJoinable(int iChannelIndex) { return m_DuelChannels[iChannelIndex].m_bJoinable; }
	char * GetDuelChannelUserID1(int iChannelIndex) { return m_DuelChannels[iChannelIndex].m_szID1; }
	char * GetDuelChannelUserID2(int iChannelIndex) { return m_DuelChannels[iChannelIndex].m_szID2; }

	void SetCurrentChannel(int iChannel = -1) { m_iCurrentChannel = iChannel; }
	int GetCurrentChannel() { return m_iCurrentChannel; }

	void RemoveAllDuelWatchUser();
	void AddDuelWatchUser(char * pszUserID);
	void RemoveDuelWatchUser(char * pszUserID);
	char * GetDuelWatchUser(int iIndex);
	int GetDuelWatchUserCount() { return m_DuelWatchUserList.size(); }

	BOOL GetFighterRegenerated() { return m_bRegenerated; }
	void SetFighterRegenerated(BOOL bFlag) { m_bRegenerated = bFlag; }

protected:
	DUEL_CHANNEL_INFO m_DuelChannels[MAX_DUEL_CHANNELS];
	int m_iCurrentChannel;

	BOOL m_bRegenerated;

	std::list<std::string> m_DuelWatchUserList;
};

extern CDuelMgr g_DuelMgr;
