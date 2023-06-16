// CSMServer
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzTexture.h"
#include "ZzzOpenglUtil.h"
#include "zzzOpenData.h"
#include "zzzinfomation.h"
#include "wsclientinline.h"
#include "CSMapServer.h"

extern int LogIn;
extern char LogInID[MAX_ID_SIZE + 1];
extern int HeroKey;

static CSMServer csMapServer;

CSMServer::CSMServer ()
{
    Init ();
}

void CSMServer::Init()
{
    m_bFillServerInfo = false;
    memset(&m_vServerInfo, 0, sizeof(MServerInfo));
}

void CSMServer::SetHeroID ( char* ID )
{
    m_strHeroID = ID;
}

void CSMServer::SetServerInfo(const MServerInfo& sInfo)
{
    memcpy(&m_vServerInfo, &sInfo, sizeof(MServerInfo));

    m_bFillServerInfo = true;
}

void CSMServer::GetServerInfo(MServerInfo& sInfo)
{
    if (m_bFillServerInfo)
    {
        memcpy(&sInfo, &m_vServerInfo, sizeof(MServerInfo));
    }
    else
    {
        memset(&sInfo, 0, sizeof(MServerInfo));
    }
}

void CSMServer::GetServerAddress(char* szAddress)
{
    if (m_bFillServerInfo)
    {
        strncpy(szAddress, m_vServerInfo.m_szMapSvrIpAddress, 16);
        szAddress[15] = '\0'; // Ensure null termination
    }
    else
    {
        memset(szAddress, 0, sizeof(char) * 16);
    }
}

void CSMServer::ConnectChangeMapServer(MServerInfo sInfo)
{
    SetServerInfo(sInfo);

    if (m_bFillServerInfo && LogIn != 0)
    {
        DeleteSocket();
        SaveOptions();
        SaveMacro("Data\\Macro.txt");

        Sleep(20);

        if (CreateSocket(m_vServerInfo.m_szMapSvrIpAddress, m_vServerInfo.m_wMapSvrPort))
        {
            g_bGameServerConnected = true;
        }
    }
}

void CSMServer::SendChangeMapServer()
{
    if (!m_bFillServerInfo || LogIn == 0)
        return;

    char CharID[MAX_ID_SIZE + 1];
    strncpy(CharID, m_strHeroID.c_str(), MAX_ID_SIZE);
    CharID[MAX_ID_SIZE] = '\0';

    ClearCharacters(-1);
    InitGame();
    SendChangeMServer(LogInID, CharID, m_vServerInfo.m_iJoinAuthCode1, m_vServerInfo.m_iJoinAuthCode2, m_vServerInfo.m_iJoinAuthCode3, m_vServerInfo.m_iJoinAuthCode4);
}