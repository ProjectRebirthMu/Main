// NewUIPartyListWindow.h: interface for the CNewUIPartyInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_NEWUIPARTYMINIWINDOW_H_)
#define _NEWUIPARTYMINIWINDOW_H_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIButton.h"
#include "NewUIPartyInfoWindow.h"

namespace SEASON3B
{
	class CNewUIPartyListWindow : public CNewUIObj  
	{
	public:
		enum IMAGE_LIST
		{	
			// ��Ÿ �̹���
			IMAGE_PARTY_LIST_FLAG = CNewUIPartyInfoWindow::IMAGE_PARTY_FLAG,		// newui_party_flag.tga			(9, 10)	
			IMAGE_PARTY_LIST_EXIT = CNewUIPartyInfoWindow::IMAGE_PARTY_EXIT,		// newui_party_x.tga			(11, 11)
				
			// ��Ƽ�̴�â
			IMAGE_PARTY_LIST_BACK = BITMAP_PARTY_MINILIST_BEGIN,					// newui_party_back.tga			(77, 23)
			IMAGE_PARTY_LIST_HPBAR,													// newui_party_hpbar.jpg		(69, 3)										
		};
	private:
		enum PARTY_WINDOW_SIZE
		{
			PARTY_LIST_WINDOW_WIDTH = 77,
			PARTY_LIST_WINDOW_HEIGHT = 23,
			PARTY_LIST_HP_BAR_WIDTH = 69,
			PARTY_LIST_HP_BAR_HEIGHT = 3,
		};

		enum PARTY_BG_COLOR
		{
			PARTY_LIST_BGCOLOR_DEFAULT = 0,
			PARTY_LIST_BGCOLOR_RED,
			PARTY_LIST_BGCOLOR_GREEN
		};
		
	private:
		CNewUIManager*				m_pNewUIMng;
		POINT						m_Pos;

		CNewUIButton				m_BtnPartyExit[MAX_PARTYS];				// ��ƼŻ�� ��ư
		int							m_iPartyListBGColor[MAX_PARTYS];		// ��Ƽ����Ʈ ���Į��
		bool						m_bPartyMemberoutofSight[MAX_PARTYS];	// ��Ƽ���� �� ĳ������ �þ� �ۿ� �ִ°�
		
		bool						m_bActive;
		int							m_iVal;		// �ε����� ���� ����(y�� ��ġ)

		int							m_iSelectedCharacter;
		int							m_iLimitUserIDHeight[2];
		
	public:
		CNewUIPartyListWindow();
		virtual ~CNewUIPartyListWindow();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		void SetPos(int x);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		bool BtnProcess();
		
		float GetLayerDepth();	//. 5.4f

		void OpenningProcess();
		void ClosingProcess();	
		
		int GetSelectedCharacter();
		void SetListBGColor( );
		
	private:
		void LoadImages();
		void UnloadImages();	
		
		bool SelectCharacterInPartyList( PARTY_t* pMember );
		void RenderPartyHPOnHead( );
	};
}

#endif // !defined(_NEWUIPARTYMINIWINDOW_H_)
