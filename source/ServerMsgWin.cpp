//*****************************************************************************
// File: ServerMsgWin.cpp
// Revised: 10/07/23
//*****************************************************************************

#include "stdafx.h"
#include "ServerMsgWin.h"
#include "UIControls.h"

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

CServerMsgWin::CServerMsgWin()
{}

CServerMsgWin::~CServerMsgWin()
{}

void CServerMsgWin::Create()
{
	SImgInfo aiiDescBg[WE_BG_MAX] =
	{
		{ BITMAP_LOG_IN + 11, 0, 0, 4, 4 },
		{ BITMAP_LOG_IN + 12, 0, 0, 512, 6 },
		{ BITMAP_LOG_IN + 12, 0, 6, 512, 6 },
		{ BITMAP_LOG_IN + 13, 0, 0, 3, 4 },
		{ BITMAP_LOG_IN + 13, 3, 0, 3, 4 }
	};
	CWinEx::Create(aiiDescBg, 1, SMW_MSG_LINE_MAX * 5);

	std::memset(m_aszMsg, 0, sizeof(char) * SMW_MSG_LINE_MAX * SMW_MSG_ROW_MAX);
	m_nMsgLine = 0;
}

bool CServerMsgWin::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)
		return false;

	switch (nArea)
	{
	case WA_ALL:
		return false;
	default:
		return CWinEx::CursorInWin(nArea);
	}
}

void CServerMsgWin::AddMsg(const char* pszMsg)
{
	if (++m_nMsgLine > SMW_MSG_LINE_MAX)
	{
		m_nMsgLine = SMW_MSG_LINE_MAX;
		for (int i = 0; i < SMW_MSG_LINE_MAX - 1; ++i)
			std::strcpy(m_aszMsg[i], m_aszMsg[i + 1]);
	}
	else
		CWinEx::SetLine(m_nMsgLine * 5);

	std::strcpy(m_aszMsg[m_nMsgLine - 1], pszMsg);

	CWinEx::Show(true);
}

void CServerMsgWin::RenderControls()
{
	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->SetBgColor(0);

	for (int i = 0; i < m_nMsgLine; ++i)
	{
		g_pRenderText->RenderText(static_cast<int>((CWin::GetXPos() + 11) / g_fScreenRate_x),
			static_cast<int>((CWin::GetYPos() + 12 + i * 20) / g_fScreenRate_y),
			m_aszMsg[i]);
	}
}
