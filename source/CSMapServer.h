//////////////////////////////////////////////////////////////////////////
//  CSMapServer.h
//////////////////////////////////////////////////////////////////////////

#ifndef __CS_MAP_SERVER_H__
#define __CS_MAP_SERVER_H__

#include "Singleton.h"

typedef struct
{
    char m_szMapSvrIpAddress[16];
    uint16_t m_wMapSvrPort;
    uint16_t m_wMapSvrCode;
    int32_t m_iJoinAuthCode1;
    int32_t m_iJoinAuthCode2;
    int32_t m_iJoinAuthCode3;
    int32_t m_iJoinAuthCode4;
} MServerInfo;

class CSMServer : public Singleton<CSMServer>
{
private:
    bool m_bFillServerInfo;
    std::string m_strHeroID;
    MServerInfo m_vServerInfo;

public:
    CSMServer ();
    ~CSMServer (){}

    void Init ( void );
    void SetHeroID ( char* ID );
    void SetServerInfo (const MServerInfo& sInfo);
    void GetServerInfo ( MServerInfo& sInfo );
    void GetServerAddress ( char* szAddress );
    WORD GetServerPort() const { return m_bFillServerInfo ? m_vServerInfo.m_wMapSvrPort : 0; }
    WORD GetServerCode() const { return m_bFillServerInfo ? m_vServerInfo.m_wMapSvrCode : 0; }
    void ConnectChangeMapServer ( MServerInfo sInfo );
    void SendChangeMapServer ( void );
};

#define g_csMapServer (CSMServer::GetSingleton())

#endif// __CS_MAP_SERVER_H__

