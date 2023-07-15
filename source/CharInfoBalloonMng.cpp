//*****************************************************************************
// File: CharInfoBalloonMng.cpp
// Revised: 10/07/23
//*****************************************************************************

#include "stdafx.h"
#include "CharInfoBalloonMng.h"
#include "CharInfoBalloon.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCharInfoBalloonMng::CCharInfoBalloonMng() : m_pCharInfoBalloon(nullptr)
{}

CCharInfoBalloonMng::~CCharInfoBalloonMng()
{
	Release();
}

void CCharInfoBalloonMng::Release()
{
	delete[] m_pCharInfoBalloon;
	m_pCharInfoBalloon = nullptr;
}

void CCharInfoBalloonMng::Create()
{
	if (m_pCharInfoBalloon == nullptr)
		m_pCharInfoBalloon = new CCharInfoBalloon[5];

	for (int i = 0; i < 5; ++i)
		m_pCharInfoBalloon[i].Create(&CharactersClient[i]);
}

void CCharInfoBalloonMng::Render()
{
	if (m_pCharInfoBalloon == nullptr)
		return;

	for (int i = 0; i < 5; ++i)
		m_pCharInfoBalloon[i].Render();
}

void CCharInfoBalloonMng::UpdateDisplay()
{
	if (m_pCharInfoBalloon == nullptr)
		return;

	for (int i = 0; i < 5; ++i)
		m_pCharInfoBalloon[i].SetInfo();
}