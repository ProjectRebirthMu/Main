// BlackWin.cpp: implementation of the CBlackWin class.
// Revised: 10/07/23
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BlackWin.h"
#include "Input.h"
#include "UIMng.h"

#define BLW_FADE_TIME	1000.0

CBlackWin::CBlackWin()
{}

CBlackWin::~CBlackWin()
{}

void CBlackWin::Create()
{
	CInput& rInput = CInput::Instance();
	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());
}

bool CBlackWin::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)
		return false;

	switch (nArea)
	{
	case WA_MOVE:
		return false;
	default:
		return CWin::CursorInWin(nArea);
	}
}

void CBlackWin::UpdateWhileActive(double dDeltaTick)
{
	short nAlpha = static_cast<short>(CWin::GetBgAlpha());
	nAlpha -= static_cast<short>(255.0 * dDeltaTick / BLW_FADE_TIME);
	if (nAlpha <= 0)
	{
		nAlpha = 0;
		CUIMng::Instance().HideWin(this);
	}
	CWin::SetBgAlpha(static_cast<BYTE>(nAlpha));
}
