// NewUIHeroPositionInfo.cpp: implementation of the CNewUIHeroPositionInfo class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIHeroPositionInfo.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "MapManager.h"

using namespace SEASON3B;

CNewUIHeroPositionInfo::CNewUIHeroPositionInfo()
{
	m_pNewUIMng = nullptr;
	m_Pos.x = m_Pos.y = 0;
	m_CurHeroPosition.x = m_CurHeroPosition.y = 0;
	WidenX = 0;
}

CNewUIHeroPositionInfo::~CNewUIHeroPositionInfo()
{
	Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIHeroPositionInfo::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (nullptr == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_HERO_POSITION_INFO, this);

	WidenX = (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH + (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH * 0.2f));
	if (WindowWidth > 800)
	{
		WidenX = (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH + (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH * 0.4f));
	}

	SetPos(x, y);
	LoadImages();

#ifdef ENABLE_MUHELPER
	unicode::t_string tooltiptext1 = GlobalText[3561];
	unicode::t_string btname1 = "";

	SetButtonInfo(
		&m_BtnConfig,
		IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 3,
		x + WidenX + 41,
		y,
		18,
		13,
		1,
		0,
		1,
		1u,
		btname1,
		tooltiptext1,
		0);

	unicode::t_string tooltiptext2 = GlobalText[3562];
	unicode::t_string btname2 = "";

	SetButtonInfo(
		&m_BtnStart,
		IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 4,
		x + WidenX + 59,
		y,
		18,
		13,
		1,
		0,
		1,
		1u,
		btname2,
		tooltiptext2,
		0);

	unicode::t_string tooltiptext3 = GlobalText[3563];
	unicode::t_string btname3 = "";

	SetButtonInfo(
		&m_BtnStop,
		IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 5,
		x + WidenX + 59,
		y,
		18,
		13,
		1,
		0,
		1,
		1u,
		btname2,
		tooltiptext3,
		0);
#endif

	MoveTextTipPos(&m_BtnConfig, -20, 9);
	MoveTextTipPos(&m_BtnStart, -20, 9);
	MoveTextTipPos(&m_BtnStop, -20, 9);

	Show(true);

	return true;
}

void CNewUIHeroPositionInfo::Release()
{
	UnloadImages();

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = nullptr;
	}
}

void CNewUIHeroPositionInfo::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUIHeroPositionInfo::BtnProcess()
{
	if (m_BtnConfig.UpdateMouseEvent())
	{
		return true;
	}
	else
	{
		if (m_BtnStart.UpdateMouseEvent())
		{
			return true;
		}
	}
	return false;
}

bool CNewUIHeroPositionInfo::UpdateMouseEvent()
{
	if (true == BtnProcess())
	{
		return false;
	}

	int Width = HERO_POSITION_INFO_BASEA_WINDOW_WIDTH + WidenX + 73;

	if (CheckMouseIn(m_Pos.x, m_Pos.y, Width, HERO_POSITION_INFO_BASE_WINDOW_HEIGHT))
	{
		return false;
	}

	return true;
}

bool CNewUIHeroPositionInfo::UpdateKeyEvent()
{
	return true;
}

bool CNewUIHeroPositionInfo::Update()
{
	if ((IsVisible() == true) && (Hero != nullptr))
	{
		m_CurHeroPosition.x = (Hero->PositionX);
		m_CurHeroPosition.y = (Hero->PositionY);
	}

	return true;
}

bool CNewUIHeroPositionInfo::Render()
{
	unicode::t_char szText[255] = { NULL, };

	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW, m_Pos.x, m_Pos.y, float(HERO_POSITION_INFO_BASEA_WINDOW_WIDTH), float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT));
	RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1, m_Pos.x + HERO_POSITION_INFO_BASEA_WINDOW_WIDTH, m_Pos.y, float(WidenX), float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT), 0.1f, 0.f, 22.4f / 32.f, 25.f / 32.f);
#ifdef ENABLE_MUHELPER
	RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2, m_Pos.x + HERO_POSITION_INFO_BASEA_WINDOW_WIDTH + WidenX, m_Pos.y, 456.f, 20.f);
#else
	RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2, m_Pos.x + HERO_POSITION_INFO_BASEA_WINDOW_WIDTH + WidenX, m_Pos.y, 22.f, 20.f);
#endif // 
	//--
	m_BtnConfig.Render();

	m_BtnStart.Render();
	//--
	unicode::_sprintf(szText, "%s (%d , %d)", gMapManager.GetMapName(gMapManager.WorldActive), m_CurHeroPosition.x, m_CurHeroPosition.y);

	g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 5, szText, WidenX + 20, 13 - 4, RT3_SORT_CENTER);
	DisableAlphaBlend();
	return true;
}

float CNewUIHeroPositionInfo::GetLayerDepth()
{
	return 4.3f;
}

void CNewUIHeroPositionInfo::OpenningProcess()
{

}

void CNewUIHeroPositionInfo::ClosingProcess()
{

}

void CNewUIHeroPositionInfo::SetCurHeroPosition(int x, int y)
{
	m_CurHeroPosition.x = x;
	m_CurHeroPosition.y = y;
}

void CNewUIHeroPositionInfo::LoadImages()
{
	LoadBitmap("Interface\\Minimap_positionA.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW, GL_LINEAR);
	LoadBitmap("Interface\\Minimap_positionB.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1, GL_LINEAR);
#ifdef ENABLE_MUHELPER
	LoadBitmap("Interface\\MacroUI\\Minimap_positionC.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2, GL_LINEAR);
	LoadBitmap("Interface\\MacroUI\\MacroUI_Setup.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 3, GL_LINEAR);
	LoadBitmap("Interface\\MacroUI\\MacroUI_Start.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 4, GL_LINEAR);
	LoadBitmap("Interface\\MacroUI\\MacroUI_Stop.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 5, GL_LINEAR);
#else
	LoadBitmap("Interface\\Minimap_positionC.tga", IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2, GL_LINEAR);
#endif // ENABLE_MUHELPER
}

void CNewUIHeroPositionInfo::UnloadImages()
{
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW);
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1);
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2);
#ifdef ENABLE_MUHELPER
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 3);
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 4);
	DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 5);
#endif // ENABLE_MUHELPER
}

void CNewUIHeroPositionInfo::SetButtonInfo(CNewUIButton* m_Btn, int imgindex, int x, int y, int sx, int sy, bool overflg, bool isimgwidth, bool bClickEffect, bool MoveTxt, unicode::t_string btname, unicode::t_string tooltiptext, bool istoppos)
{
	m_Btn->ChangeButtonImgState(1, imgindex, overflg, isimgwidth, bClickEffect);
	m_Btn->ChangeButtonInfo(x, y, sx, sy);

	m_Btn->ChangeText(btname);
	m_Btn->ChangeToolTipText(tooltiptext, istoppos);

	if (MoveTxt)
	{
		m_Btn->MoveTextPos(0, -1);
	}
}

void CNewUIHeroPositionInfo::MoveTextTipPos(CNewUIButton* m_Btn, int iX, int iY)
{
	m_Btn->MoveTextTipPos(iX, iY);
}
