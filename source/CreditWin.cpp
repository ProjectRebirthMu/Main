//*****************************************************************************
// File: CreditWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CreditWin.h"
#include "Input.h"
#include "UIMng.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "DSPlaySound.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "Local.h"
#include "./Utilities/Log/ErrorReport.h"
#include "wsclientinline.h"
#include "UIControls.h"

#define	CRW_ILLUST_FADE_TIME	2000.0
#define	CRW_ILLUST_SHOW_TIME	22000.0

#define	CRW_TEXT_FADE_TIME		300.0
#define	CRW_NAME_SHOW_TIME		2300.0

#define	CRW_DATA_FILE		"Data\\Local\\credit.bmd"

extern float g_fScreenRate_x;
extern char* g_lpszMp3[NUM_MUSIC];

CCreditWin::CCreditWin() : m_hFont(NULL)
{
}

CCreditWin::~CCreditWin()
{

}

void CCreditWin::Create()
{
	// Get the instance of CInput
	CInput& rInput = CInput::Instance();

	// Create the window with the dimensions obtained from CInput
	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());
	CWin::SetBgAlpha(255);

	// Calculate scaling factors for images and text
	float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
	float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;

	// Create sprite for decorative image and logo image
	m_aSpr[CRW_SPR_DECO].Create(189, 103, BITMAP_LOG_IN + 6);
	m_aSpr[CRW_SPR_LOGO].Create(290, 41, BITMAP_LOG_IN + 14, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);

	// Create sprites for text
	for (int i = CRW_SPR_TXT_HIDE0; i <= CRW_SPR_TXT_HIDE2; ++i)
	{
		m_aSpr[i].Create(800, 42, -1, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
		m_aSpr[i].SetColor(0, 0, 0);
	}

	// Create close button and register it with the window
	m_btnClose.Create(54, 30, BITMAP_BUTTON + 2, 3, 2, 1);
	CWin::RegisterButton(&m_btnClose);

	// Set the illustration state to HIDE and specify paths for illustration images
	m_eIllustState = HIDE;
	char* apszIllustPath[8][2] = {
		{"Interface\\im1_1.jpg", "Interface\\im1_2.jpg"},
		{"Interface\\im2_1.jpg", "Interface\\im2_2.jpg"},
		{"Interface\\im3_1.jpg", "Interface\\im3_2.jpg"},
		{"Interface\\im4_1.jpg", "Interface\\im4_2.jpg"},
		{"Interface\\im5_1.jpg", "Interface\\im5_2.jpg"},
		{"Interface\\im6_1.jpg", "Interface\\im6_2.jpg"},
		{"Interface\\im7_1.jpg", "Interface\\im7_2.jpg"},
		{"Interface\\im8_1.jpg", "Interface\\im8_2.jpg"}
	};

	std::copy_n(&apszIllustPath[0][0], 16, &m_apszIllustPath[0][0]);

	// Set font size based on screen width and create the font
	int nFontSize = 10;
	switch (rInput.GetScreenWidth())
	{
	case 640: nFontSize = 10; break;
	case 800: nFontSize = 12; break;
	case 1024: nFontSize = 13; break;
	case 1280: nFontSize = 13; break;
	case 1360: nFontSize = 14; break;
	case 1440: nFontSize = 16; break;
	case 1600: nFontSize = 16; break;
	case 1680: nFontSize = 16; break;
	case 1920: nFontSize = 18; break;
	}
	m_hFont = CreateFont(nFontSize, 0, 0, 0, FW_BOLD, 0, 0, 0,DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,GlobalText[0][0] ? GlobalText[0] : NULL);

	LoadText();
	SetPosition();
}

void CCreditWin::PreRelease()
{
	for (int i = 0; i < CRW_SPR_MAX; ++i)
		m_aSpr[i].Release();

	if (m_hFont)
	{
		::DeleteObject((HGDIOBJ)m_hFont);
		m_hFont = NULL;
	}
}

void CCreditWin::SetPosition()
{
	m_aSpr[CRW_SPR_PIC_L].SetPosition(0, 126);
	m_aSpr[CRW_SPR_PIC_R].SetPosition(400, 126);
	m_aSpr[CRW_SPR_LOGO].SetPosition(241, 549);


	CInput rInput = CInput::Instance();

	int nBaseY = int(527.0f / 600.0f * (float)rInput.GetScreenHeight());
	m_aSpr[CRW_SPR_DECO].SetPosition(rInput.GetScreenWidth() - m_aSpr[CRW_SPR_DECO].GetWidth(),	nBaseY - m_aSpr[CRW_SPR_DECO].GetHeight());

	for (int i = CRW_SPR_TXT_HIDE0; i <= CRW_SPR_TXT_HIDE2; ++i)
		m_aSpr[i].SetPosition(0, 42 * (i - CRW_SPR_TXT_HIDE0));

	m_btnClose.SetPosition(m_aSpr[CRW_SPR_DECO].GetXPos() + 122,
		m_aSpr[CRW_SPR_DECO].GetYPos() + 63);
}

void CCreditWin::Show(bool bShow)
{
	CWin::Show(bShow);

	for (int i = 0; i < CRW_SPR_MAX; ++i)
		m_aSpr[i].Show(bShow);

	m_btnClose.Show(bShow);

	if (bShow)
		Init();
	else
		m_eIllustState = HIDE;
}

bool CCreditWin::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)
		return false;

	switch (nArea)
	{
	case WA_MOVE:
		return false;
	}

	return CWin::CursorInWin(nArea);
}

void CCreditWin::UpdateWhileActive(double dDeltaTick)
{
	if (m_btnClose.IsClick())
		CloseWin();
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		::PlayBuffer(SOUND_CLICK01);
		CloseWin();
		CUIMng::Instance().SetSysMenuWinShow(false);
	}

	for (int i = 0; i <= CRW_INDEX_NAME; ++i)
		AnimationText(i, dDeltaTick);
	AnimationIllust(dDeltaTick);
}

void CCreditWin::RenderControls()
{
	::glDisable(GL_ALPHA_TEST);

	for (int i = 0; i <= CRW_SPR_LOGO; ++i)
		m_aSpr[i].Render();

	long lScreenWidth = CInput::Instance().GetScreenWidth();
	int nTextBoxWidth;

	g_pRenderText->SetFont(m_hFont);
	g_pRenderText->SetTextColor(CLRDW_BR_GRAY);
	g_pRenderText->SetBgColor(0);
	
	nTextBoxWidth = lScreenWidth / g_fScreenRate_x;

	g_pRenderText->RenderText(0, 20,
		m_aCredit[m_anTextIndex[CRW_INDEX_DEPARTMENT]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
	
	g_pRenderText->RenderText(0, 46, m_aCredit[m_anTextIndex[CRW_INDEX_TEAM]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);

	g_pRenderText->SetTextColor(CLRDW_BR_YELLOW);
	
	switch (m_nNameCount)
	{
	case 1:
		g_pRenderText->RenderText(0, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
		break;
	case 2:
		nTextBoxWidth = lScreenWidth / 4 / g_fScreenRate_x;
		g_pRenderText->RenderText(160, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(320, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME1]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
		break;
	case 3:
		nTextBoxWidth = lScreenWidth / 3 / g_fScreenRate_x;
		g_pRenderText->RenderText(0, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(213, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME1]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(426, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME2]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
		break;
	case 4:
		nTextBoxWidth = lScreenWidth / 4 / g_fScreenRate_x;
		g_pRenderText->RenderText(0, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(160, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME1]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(320, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME2]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(480, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME3]].szName,nTextBoxWidth, 0, RT3_SORT_CENTER);
		break;
	}

	for (int i = CRW_SPR_TXT_HIDE0; i <= CRW_SPR_TXT_HIDE2; ++i)
		m_aSpr[i].Render();

	glEnable(GL_ALPHA_TEST);

	CWin::RenderButtons();
}

void CCreditWin::CloseWin()
{
	CUIMng::Instance().HideWin(this);

	SendRequestServerList();

	::StopMp3(g_lpszMp3[MUSIC_MUTHEME]);
	::PlayMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
}

void CCreditWin::Init()
{
	m_eIllustState = FADEIN;
	m_dIllustDeltaTickSum = 0.0;
	m_byIllust = 0;
	LoadIllust();

	for (int i = 0; i <= CRW_INDEX_NAME; ++i)
		m_aeTextState[i] = FADEIN;
	m_dTextDeltaTickSum = 0.0;
	m_nNowIndex = 0;
	m_nNameCount = 0;
	SetTextIndex();
}

void CCreditWin::LoadIllust()
{
	CInput rInput = CInput::Instance();
	float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
	float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;
	
	for (int i = 0; i < 2; ++i)
	{
		LoadBitmap(m_apszIllustPath[m_byIllust][i], BITMAP_TEMP+i, GL_LINEAR);
		
		m_aSpr[i].Create(400, 400, BITMAP_TEMP+i, 0, NULL, 0, 0,
			false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
		m_aSpr[i].SetAlpha(0);
		m_aSpr[i].Show(true);
	}

	m_aSpr[CRW_SPR_PIC_L].SetPosition(0, 126);
	m_aSpr[CRW_SPR_PIC_R].SetPosition(400, 126);
}


void CCreditWin::AnimationIllust(double dDeltaTick)
{
	short nAlpha;
	switch (m_eIllustState)
	{
	case FADEIN:
		nAlpha = short(m_aSpr[CRW_SPR_PIC_L].GetAlpha());
		nAlpha += short(255.0 * dDeltaTick / CRW_ILLUST_FADE_TIME);
		if (255 <= nAlpha)
		{
			m_eIllustState = SHOW;
			nAlpha = 255;
		}
		m_aSpr[CRW_SPR_PIC_L].SetAlpha((BYTE)nAlpha);
		m_aSpr[CRW_SPR_PIC_R].SetAlpha((BYTE)nAlpha);
		break;

	case SHOW:
		m_dIllustDeltaTickSum += dDeltaTick;
		if (m_dIllustDeltaTickSum > CRW_ILLUST_SHOW_TIME)
		{
			m_eIllustState = FADEOUT;
			m_dIllustDeltaTickSum = 0.0;
		}
		break;

	case FADEOUT:
		nAlpha = short(m_aSpr[CRW_SPR_PIC_L].GetAlpha());
		nAlpha -= short(255.0 * dDeltaTick / CRW_ILLUST_FADE_TIME);
		if (0 >= nAlpha)
		{
			m_eIllustState = FADEIN;
			nAlpha = 0;

			m_byIllust = ++m_byIllust == CRW_ILLUST_MAX ? 0 : m_byIllust;
			LoadIllust();
		}
		m_aSpr[CRW_SPR_PIC_L].SetAlpha((BYTE)nAlpha);
		m_aSpr[CRW_SPR_PIC_R].SetAlpha((BYTE)nAlpha);
		break;
	}
}

void BuxConvert(BYTE *Buffer, int Size);

void CCreditWin::LoadText()
{
	FILE *fp = ::fopen(CRW_DATA_FILE, "rb");
	if (fp == NULL)
	{
		char szMessage[256];
		::sprintf(szMessage, "%s file not found.\r\n", CRW_DATA_FILE);
		g_ErrorReport.Write(szMessage);
		::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
		::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
		return;
	}

	int nSize = sizeof(SCreditItem) * CRW_ITEM_MAX;
	::fread(m_aCredit, nSize, 1, fp);
	::BuxConvert((BYTE*)m_aCredit, nSize);

	::fclose(fp);
}

void CCreditWin::SetTextIndex()
{
	if (0 == m_aCredit[m_nNowIndex].byClass)
	{
		::PlayBuffer(SOUND_CLICK01);
		CloseWin();
	}

	if (1 == m_aCredit[m_nNowIndex].byClass)
	{
		m_anTextIndex[CRW_INDEX_DEPARTMENT] = m_nNowIndex;
		++m_nNowIndex;
	}
	if (2 == m_aCredit[m_nNowIndex].byClass)
	{
		m_anTextIndex[CRW_INDEX_TEAM] = m_nNowIndex;
		++m_nNowIndex;
	}

	int iNameCnt = 0;
	for (int i = 0; i < 4; ++i)
	{
		iNameCnt = i;
		if (3 == m_aCredit[m_nNowIndex].byClass)
		{
			m_anTextIndex[CRW_INDEX_NAME0+i] = m_nNowIndex;
			++m_nNowIndex;
		}
		else
			break;
	}
	m_nNameCount = iNameCnt;
}

void CCreditWin::AnimationText(int nClass, double dDeltaTick)
{
	SHOW_STATE* peTextState = &m_aeTextState[nClass];
	short nAlpha;

	CSprite* psprHide = &m_aSpr[CRW_SPR_TXT_HIDE0 + nClass];

	switch (*peTextState)
	{
	case FADEIN:
		nAlpha = short(psprHide->GetAlpha());
		nAlpha -= short(255.0 * dDeltaTick / CRW_TEXT_FADE_TIME);
		if (0 >= nAlpha)
		{
			*peTextState = SHOW;
			nAlpha = 0;
		}
		psprHide->SetAlpha((BYTE)nAlpha);
		break;

	case SHOW:
		if (nClass != CRW_INDEX_NAME)
			break;

		m_dTextDeltaTickSum += dDeltaTick;
		if (m_dTextDeltaTickSum > CRW_NAME_SHOW_TIME)
		{
			m_aeTextState[CRW_INDEX_NAME] = FADEOUT;
			m_dTextDeltaTickSum = 0.0;

			if (3 != m_aCredit[m_nNowIndex].byClass)
			{
				m_aeTextState[CRW_INDEX_TEAM] = FADEOUT;
				if (2 != m_aCredit[m_nNowIndex].byClass)
					m_aeTextState[CRW_INDEX_DEPARTMENT] = FADEOUT;
			}
		}
		break;

	case FADEOUT:
		nAlpha = short(psprHide->GetAlpha());
		nAlpha += short(255.0 * dDeltaTick / CRW_TEXT_FADE_TIME);
		if (255 <= nAlpha)
		{
			*peTextState = FADEIN;
			nAlpha = 255;

			if (nClass == CRW_INDEX_NAME)
				SetTextIndex();
		}
		psprHide->SetAlpha((BYTE)nAlpha);
		break;
	}
}
